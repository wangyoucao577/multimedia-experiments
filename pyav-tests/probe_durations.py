import av
import argparse


def timestamp2ms(ts, time_base):
    return float(ts) * time_base * 1000


def format_timestamp_in_ms(ts, time_base):
    if ts is None or time_base is None:
        return "none"

    ts_in_ms = timestamp2ms(ts, time_base)
    return "{:.3f} ms".format(ts_in_ms)


class StreamInfo:
    stream = None
    time_base = None

    first_dts = None
    first_pts = None

    last_dts = None
    last_pts = None
    last_duration = None

    first_valid_packet_dts = None  # once pts >= 0
    first_valid_packet_pts = None  # once pts >= 0

    smallest_valid_pts = None  # pts >= 0 and the smallest one
    biggest_pts = None
    biggest_pts_packet_duration = None

    # calculations
    duration_by_sum = 0
    duration_by_dts = 0
    duration_by_pts = 0
    duration_by_pts_and_start_time = 0
    duration_delta_by_sum = None
    duration_delta_by_dts = None
    duration_delta_by_pts = None
    duration_delta_by_pts_and_start_time = None

    def __init__(self, stream):
        self.stream = stream

    def calculate_durations(self):
        # calc duration by dts/pts
        self.duration_by_dts = self.last_dts - self.first_dts
        self.duration_by_pts = self.biggest_pts - self.smallest_valid_pts
        self.duration_by_pts_and_start_time = self.biggest_pts - self.stream.start_time
        if self.last_duration is not None:
            self.duration_by_dts += self.last_duration
        if self.biggest_pts_packet_duration is not None:
            self.duration_by_pts += self.biggest_pts_packet_duration
            self.duration_by_pts_and_start_time += self.biggest_pts_packet_duration

        # calc delta between calculated duration and stream duration
        if self.stream.duration is not None:
            self.duration_delta_by_sum = self.duration_by_sum - self.stream.duration
            self.duration_delta_by_dts = self.duration_by_dts - self.stream.duration
            self.duration_delta_by_pts = self.duration_by_pts - self.stream.duration
            self.duration_delta_by_pts_and_start_time = (
                self.duration_by_pts_and_start_time - self.stream.duration
            )

    def print_timestamp(self, ts, name, prefix="  ", suffix=""):
        ts_in_ms = format_timestamp_in_ms(ts, self.time_base)
        print(f"{prefix}{name} {ts} ({ts_in_ms}){suffix}")

    def dump(self):
        print(f"stream[{self.stream.index}] {self.stream.type}")
        print(f"  profile {self.stream.profile}")
        print(f"  frames {self.stream.frames}")
        if (
            self.stream.average_rate is not None
            or self.stream.base_rate is not None
            or self.stream.guessed_rate is not None
        ):
            print(f"  avg_frame_rate {self.stream.average_rate}")
            print(f"  r_frame_rate {self.stream.base_rate}")
            print(f"  guessed_frame_rate {self.stream.guessed_rate}")
        print(f"  time_base {self.stream.time_base}")

        self.print_timestamp(self.stream.start_time, "start_time")
        self.print_timestamp(self.stream.duration, "duration")

        # print calculated durations
        self.print_timestamp(
            self.duration_by_sum,
            "duration_by_sum",
            suffix="    # = sum(duration of every packet)",
        )
        self.print_timestamp(
            self.duration_by_dts,
            "duration_by_dts",
            suffix="    # = last_dts - first_dts + last_duration",
        )
        self.print_timestamp(
            self.duration_by_pts,
            "duration_by_pts",
            suffix="    # = biggest_pts - smallest_valid_pts + biggest_pts_packet_duration (the most confident duration)",
        )
        self.print_timestamp(
            self.duration_by_pts_and_start_time,
            "duration_by_pts_and_start_time",
            suffix="    # = biggest_pts - start_time + biggest_pts_packet_duration",
        )
        self.print_timestamp(
            self.duration_delta_by_sum,
            "duration_delta_by_sum",
            suffix="    # = duration_by_sum - duration",
        )
        self.print_timestamp(
            self.duration_delta_by_dts,
            "duration_delta_by_dts",
            suffix="    # = duration_by_dts - duration",
        )
        self.print_timestamp(
            self.duration_delta_by_pts,
            "duration_delta_by_pts",
            suffix="    # = duration_by_pts - duration",
        )
        self.print_timestamp(
            self.duration_delta_by_pts_and_start_time,
            "duration_delta_by_pts_and_start_time",
            suffix="    # = duration_by_pts_and_start_time - duration",
        )

        # print dts/pts/duration
        self.print_timestamp(self.first_dts, "first_dts")
        self.print_timestamp(self.first_pts, "first_pts")
        self.print_timestamp(self.last_dts, "last_dts")
        self.print_timestamp(self.last_pts, "last_pts")
        self.print_timestamp(self.last_duration, "last_duration")

        self.print_timestamp(
            self.first_valid_packet_dts,
            "first_valid_packet_dts",
            suffix="    # pts >= 0",
        )
        self.print_timestamp(
            self.first_valid_packet_pts,
            "first_valid_packet_pts",
            suffix="    # pts >= 0",
        )

        self.print_timestamp(
            self.smallest_valid_pts, "smallest_valid_pts", suffix="    # pts >= 0"
        )
        self.print_timestamp(self.biggest_pts, "biggest_pts")
        self.print_timestamp(
            self.biggest_pts_packet_duration, "biggest_pts_packet_duration"
        )

    def validate_duration(self):
        if self.stream.duration is None:
            return

        # print(f"stream[{self.stream.index}] {self.stream.type}")
        # self.print_timestamp(
        #     self.duration_delta_by_pts,
        #     "duration_delta_by_pts",
        #     suffix="    # the most confident",
        # )


def main():
    parser = argparse.ArgumentParser(description="calculate duraitons.")
    parser.add_argument("-i", required=True, help="input file url", dest="input")
    parser.add_argument(
        "--disable-dump",
        required=False,
        default=True,
        action="store_false",
        help="dump basic and calculated durations",
        dest="dump",
    )
    args = parser.parse_args()
    # print(args)

    with av.open(args.input, options={"fflags": "discardcorrupt"}) as container:
        # print(container.flags)
        if args.dump:
            print(f"container")
            print(f"  duration {container.duration}")
            print(f"  start_time {container.start_time}")
            print(f"  size {container.size}")
            print(f"  bit_rate {container.bit_rate}")

        streams_info = []
        for stream in container.streams:
            # print(stream)
            streams_info.append(StreamInfo(stream))

        for packet in container.demux():
            # print(packet)
            if packet.size == 0:
                continue  # empty packet for flushing, ignore it

            si = streams_info[packet.stream_index]
            if si.time_base is None and packet.time_base is not None:
                si.time_base = packet.time_base

            if si.first_dts is None and packet.dts is not None:
                si.first_dts = packet.dts
            if si.first_pts is None and packet.pts is not None:
                si.first_pts = packet.pts

            if packet.dts is not None:
                si.last_dts = packet.dts
            if packet.pts is not None:
                si.last_pts = packet.pts
            if packet.duration is not None:
                si.last_duration = packet.duration
                si.duration_by_sum += packet.duration

            if packet.dts is not None and packet.pts is not None and packet.pts >= 0:
                if si.first_valid_packet_pts is None:
                    si.first_valid_packet_dts = packet.dts
                if si.first_valid_packet_pts is None:
                    si.first_valid_packet_pts = packet.pts
                if si.smallest_valid_pts is None or si.smallest_valid_pts >= packet.pts:
                    si.smallest_valid_pts = packet.pts
                if si.biggest_pts is None or si.biggest_pts < packet.pts:
                    si.biggest_pts = packet.pts
                    si.biggest_pts_packet_duration = packet.duration

        for si in streams_info:
            si.calculate_durations()

        if args.dump:
            for si in streams_info:
                si.dump()
        if args.validate:
            for si in streams_info:
                si.validate_duration()


if __name__ == "__main__":
    main()

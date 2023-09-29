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

    duration_by_sum = 0

    # calculations
    duration_by_dts = 0
    duration_by_pts = 0
    duration_by_pts_and_start_time = 0
    duration_delta = None

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
        # use the most confident algorithm known so far
        if self.stream.duration is not None:
            self.duration_delta = (
                self.duration_by_pts_and_start_time - self.stream.duration
            )

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

        duration_in_ms = format_timestamp_in_ms(self.stream.duration, self.time_base)
        start_time_in_ms = format_timestamp_in_ms(
            self.stream.start_time, self.time_base
        )
        print(f"  time_base {self.stream.time_base}")
        print(f"  start_time {self.stream.start_time} ({start_time_in_ms})")
        print(f"  duration {self.stream.duration} ({duration_in_ms})")

        duration_by_sum_in_ms = format_timestamp_in_ms(
            self.duration_by_sum, self.time_base
        )
        duration_by_dts_in_ms = format_timestamp_in_ms(
            self.duration_by_dts, self.time_base
        )
        duration_by_pts_in_ms = format_timestamp_in_ms(
            self.duration_by_pts, self.time_base
        )
        duration_by_pts_and_start_time_in_ms = format_timestamp_in_ms(
            self.duration_by_pts_and_start_time, self.time_base
        )
        duration_delta_in_ms = format_timestamp_in_ms(
            self.duration_delta, self.time_base
        )
        print(
            f"  duration_by_sum {self.duration_by_sum} ({duration_by_sum_in_ms})   # = sum(duration of every packet)"
        )
        print(
            f"  duration_by_dts {self.duration_by_dts} ({duration_by_dts_in_ms})   # = last_dts - first_dts + last_duration"
        )
        print(
            f"  duration_by_pts {self.duration_by_pts} ({duration_by_pts_in_ms})   # = biggest_pts - smallest_valid_pts + biggest_pts_packet_duration"
        )
        print(
            f"  duration_by_pts_and_start_time {self.duration_by_pts_and_start_time} ({duration_by_pts_and_start_time_in_ms})   # = biggest_pts - start_time + biggest_pts_packet_duration"
        )
        print(
            f"  duration_delta {self.duration_delta} ({duration_delta_in_ms})   # = duration_by_pts_and_start_time - duration"
        )

        first_dts_in_ms = format_timestamp_in_ms(self.first_dts, self.time_base)
        first_pts_in_ms = format_timestamp_in_ms(self.first_pts, self.time_base)
        last_dts_in_ms = format_timestamp_in_ms(self.last_dts, self.time_base)
        last_pts_in_ms = format_timestamp_in_ms(self.last_pts, self.time_base)
        last_duration_in_ms = format_timestamp_in_ms(self.last_duration, self.time_base)
        print(f"  first_dts {self.first_dts} ({first_dts_in_ms})")
        print(f"  first_pts {self.first_pts} ({first_pts_in_ms})")
        print(f"  last_dts {self.last_dts} ({last_dts_in_ms})")
        print(f"  last_pts {self.last_pts} ({last_pts_in_ms})")
        print(f"  last_duration {self.last_duration} ({last_duration_in_ms})")

        first_valid_packet_dts_in_ms = format_timestamp_in_ms(
            self.first_valid_packet_dts, self.time_base
        )
        first_valid_packet_pts_in_ms = format_timestamp_in_ms(
            self.first_valid_packet_pts, self.time_base
        )
        print(
            f"  first_valid_packet_dts {self.first_valid_packet_dts} ({first_valid_packet_dts_in_ms})"
        )
        print(
            f"  first_valid_packet_pts {self.first_valid_packet_pts} ({first_valid_packet_pts_in_ms})"
        )

        smallest_valid_pts_in_ms = format_timestamp_in_ms(
            self.smallest_valid_pts, self.time_base
        )
        biggest_pts_in_ms = format_timestamp_in_ms(self.biggest_pts, self.time_base)
        biggest_pts_packet_duration_in_ms = format_timestamp_in_ms(
            self.biggest_pts_packet_duration, self.time_base
        )
        print(
            f"  smallest_valid_pts {self.smallest_valid_pts} ({smallest_valid_pts_in_ms})"
        )
        print(f"  biggest_pts {self.biggest_pts} ({biggest_pts_in_ms})")
        print(
            f"  biggest_pts_packet_duration {self.biggest_pts_packet_duration} ({biggest_pts_packet_duration_in_ms})"
        )

    def validate_duration(self):
        if self.duration_delta is None:
            return

        duration_delta_in_ms = timestamp2ms(self.duration_delta, self.time_base)
        if abs(duration_delta_in_ms) >= 1:  # more than 1 ms
            print(
                f"stream[{self.stream.index}] {self.stream.type}  duration_delta_in_ms abs({duration_delta_in_ms}) >= 1 ms, invalid"
            )
            assert False
        else:
            print(
                f"stream[{self.stream.index}] {self.stream.type}  duration_delta_in_ms {duration_delta_in_ms}, valid"
            )


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
    parser.add_argument(
        "--disable-validate",
        required=False,
        default=True,
        action="store_false",
        help="validate durations",
        dest="validate",
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

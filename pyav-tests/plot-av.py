import av
import argparse
import matplotlib.pyplot as plt
import os


def timestamp2ms(ts, time_base):
    return float(ts) * time_base * 1000


class StreamInfo:
    def __init__(self, stream):
        # self.stream = stream
        self.stream_index = stream.index
        self.time_base = stream.time_base
        self.stream_type = stream.type

        self.dts_array = []
        self.pts_array = []
        self.duration_array = []
        self.size_array = []

        self.dts_delta_array = []  # dts - prev_dts

    def get_ts_array_in_ms(self, timestamps):
        return [timestamp2ms(t, self.time_base) for t in timestamps]

    def capture_by_packet(self, packet):
        prev_dts = None
        if self.dts_array:
            prev_dts = self.dts_array[-1]

        self.dts_array.append(packet.dts)
        self.pts_array.append(packet.pts)
        if packet.duration is not None:
            self.duration_array.append(packet.duration)
        self.size_array.append(packet.size)

        if prev_dts is not None:
            dts_delta = packet.dts - prev_dts
            self.dts_delta_array.append(dts_delta)


def plot_av(window_title, v_stream, a_stream):
    # line fmt
    VIDEO_LINE_FMT = "y^"
    AUDIO_LINE_FMT = "b+"

    # create axis
    fig, axs = plt.subplots(ncols=3, nrows=3, layout="constrained")
    fig.canvas.manager.set_window_title(window_title)

    # dts
    axs[0, 0].set_title(f"dts")
    axs[0, 0].set_xlabel("packet no.", loc="right")
    axs[0, 0].set_ylabel("dts (ms)")
    axs[0, 0].plot(
        v_stream.get_ts_array_in_ms(v_stream.dts_array),
        # v_stream.get_ts_array_in_ms(v_stream.dts_array),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[0, 0].plot(
        a_stream.get_ts_array_in_ms(a_stream.dts_array),
        # a_stream.get_ts_array_in_ms(a_stream.dts_array),
        AUDIO_LINE_FMT,
        label="audio",
    )
    axs[0, 0].legend()
    # axs[0, 0].legend(loc="upper left", bbox_to_anchor=(0.0, 1.02, 0.0, 0.102), ncols=2)

    # pts
    axs[0, 1].set_title(f"pts")
    axs[0, 1].set_xlabel("packet no.", loc="right")
    axs[0, 1].set_ylabel("pts (ms)")
    # axs[0, 1].yaxis.tick_right()
    # axs[0, 1].yaxis.set_label_position("right")
    axs[0, 1].plot(
        v_stream.get_ts_array_in_ms(v_stream.pts_array),
        # v_stream.get_ts_array_in_ms(v_sorted_pts_array),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[0, 1].plot(
        a_stream.get_ts_array_in_ms(a_stream.pts_array),
        # a_stream.get_ts_array_in_ms(a_sorted_pts_array),
        AUDIO_LINE_FMT,
        label="audio",
    )
    # axs[0, 1].legend()

    # size
    axs[0, 2].set_title(f"size")
    axs[0, 2].set_xlabel("packet no.", loc="right")
    axs[0, 2].set_ylabel("size (KB)")
    axs[0, 2].plot(
        [float(s) / 1024 for s in v_stream.size_array],
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[0, 2].plot(
        [float(s) / 1024 for s in a_stream.size_array],
        AUDIO_LINE_FMT,
        label="audio",
    )
    axs[0, 2].set_ylim(0)

    # dts delta
    axs[1, 0].set_title(f"dts delta")
    axs[1, 0].set_xlabel("dts (ms)", loc="right")
    axs[1, 0].set_ylabel("dts_delta (ms)")
    axs[1, 0].plot(
        v_stream.get_ts_array_in_ms(v_stream.dts_array)[1:],
        v_stream.get_ts_array_in_ms(v_stream.dts_delta_array),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[1, 0].plot(
        a_stream.get_ts_array_in_ms(a_stream.dts_array)[1:],
        a_stream.get_ts_array_in_ms(a_stream.dts_delta_array),
        AUDIO_LINE_FMT,
        label="audio",
    )
    # axs[1, 0].legend()

    # duration
    axs[1, 1].set_title(f"duration")
    axs[1, 1].set_xlabel("dts (ms)", loc="right")
    axs[1, 1].set_ylabel("duration (ms)")
    # axs[1, 1].yaxis.tick_right()
    # axs[1, 1].yaxis.set_label_position("right")
    axs[1, 1].plot(
        v_stream.get_ts_array_in_ms(v_stream.dts_array),
        v_stream.get_ts_array_in_ms(v_stream.duration_array),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[1, 1].plot(
        a_stream.get_ts_array_in_ms(a_stream.dts_array),
        a_stream.get_ts_array_in_ms(a_stream.duration_array),
        AUDIO_LINE_FMT,
        label="audio",
    )
    # axs[1, 1].legend()

    # fig.align_labels()
    plt.show()


def main():
    parser = argparse.ArgumentParser(description="plot timestamps.")
    parser.add_argument("-i", required=True, help="input file url", dest="input")
    args = parser.parse_args()
    # print(args)

    # retrieve info
    streams_info = []
    with av.open(args.input) as container:
        for stream in container.streams:
            streams_info.append(StreamInfo(stream))
        for packet in container.demux():
            # print(packet)
            if packet.size == 0:
                continue  # empty packet for flushing, ignore it
            si = streams_info[packet.stream_index]
            si.capture_by_packet(packet)

    # select v/a stream
    v_stream = None
    a_stream = None
    for s in streams_info:
        if v_stream is None and s.stream_type == "video":
            v_stream = s
        if a_stream is None and s.stream_type == "audio":
            a_stream = s

    # plot V/A timestamps
    plot_av(os.path.basename(args.input), v_stream, a_stream)


if __name__ == "__main__":
    main()

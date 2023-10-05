import av
import argparse
import matplotlib.pyplot as plt
import os
import numpy as np


class StreamInfo:
    def __init__(self, stream):
        # self.stream = stream
        self.stream_index = stream.index
        self.stream_type = stream.type
        self.time_base = stream.time_base

        # raw data from packets
        self.dts_array = []
        self.pts_array = []
        self.duration_array = []
        self.size_array = []

        self.data_array = None

    def capture_by_packet(self, packet):
        self.dts_array.append(packet.dts)
        self.pts_array.append(packet.pts)
        self.duration_array.append(packet.duration)
        self.size_array.append(packet.size)

    def finalize(self):
        # construct data array
        self.data_array = np.array(
            [
                self.dts_array,
                self.pts_array,
                self.duration_array,
                self.size_array,
                self.dts_array,  # for dts_delta calculation
            ],
            dtype=np.float64,
        )

        # calculate dts_delta = dts - prev_dts
        self.data_array[4] = np.roll(self.data_array[4], -1)
        self.data_array[4][-1] = np.nan  # ignore last value
        self.data_array[4] = self.data_array[4] - self.data_array[0]

        # [[dts, pts, duration, size], [dts, pts, duration, size], [...]]
        # self.data_array.transpose()

    def dts_array_in_seconds(self):
        return self.data_array[0] * self.time_base

    def pts_array_in_seconds(self):
        return self.data_array[1] * self.time_base

    def duration_array_in_seconds(self):
        return self.data_array[2] * self.time_base

    def size_array_in_KB(self):
        return self.data_array[3] / 1024.0

    def dts_delta_in_seconds(self):
        return self.data_array[4] * self.time_base


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
    axs[0, 0].set_ylabel("dts (s)")
    axs[0, 0].plot(
        v_stream.dts_array_in_seconds(),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[0, 0].plot(
        a_stream.dts_array_in_seconds(),
        AUDIO_LINE_FMT,
        label="audio",
    )
    axs[0, 0].legend()
    # axs[0, 0].legend(loc="upper left", bbox_to_anchor=(0.0, 1.02, 0.0, 0.102), ncols=2)

    # pts
    axs[0, 1].set_title(f"pts")
    axs[0, 1].set_xlabel("packet no.", loc="right")
    axs[0, 1].set_ylabel("pts (s)")
    # axs[0, 1].yaxis.tick_right()
    # axs[0, 1].yaxis.set_label_position("right")
    axs[0, 1].plot(
        v_stream.pts_array_in_seconds(),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[0, 1].plot(
        a_stream.pts_array_in_seconds(),
        AUDIO_LINE_FMT,
        label="audio",
    )
    # axs[0, 1].legend()

    # size
    axs[0, 2].set_title(f"size")
    axs[0, 2].set_xlabel("packet no.", loc="right")
    axs[0, 2].set_ylabel("size (KB)")
    axs[0, 2].plot(
        v_stream.size_array_in_KB(),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[0, 2].plot(
        a_stream.size_array_in_KB(),
        AUDIO_LINE_FMT,
        label="audio",
    )
    axs[0, 2].set_ylim(0)

    # dts delta
    axs[1, 0].set_title(f"dts delta")
    axs[1, 0].set_xlabel("dts (s)", loc="right")
    axs[1, 0].set_ylabel("dts_delta (s)")
    axs[1, 0].plot(
        v_stream.dts_array_in_seconds(),
        v_stream.dts_delta_in_seconds(),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[1, 0].plot(
        a_stream.dts_array_in_seconds(),
        a_stream.dts_delta_in_seconds(),
        AUDIO_LINE_FMT,
        label="audio",
    )
    # axs[1, 0].legend()

    # duration
    axs[1, 1].set_title(f"duration")
    axs[1, 1].set_xlabel("dts (s)", loc="right")
    axs[1, 1].set_ylabel("duration (s)")
    # axs[1, 1].yaxis.tick_right()
    # axs[1, 1].yaxis.set_label_position("right")
    axs[1, 1].plot(
        v_stream.dts_array_in_seconds(),
        v_stream.duration_array_in_seconds(),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[1, 1].plot(
        a_stream.dts_array_in_seconds(),
        a_stream.duration_array_in_seconds(),
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

    # process data
    for s in streams_info:
        s.finalize()

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

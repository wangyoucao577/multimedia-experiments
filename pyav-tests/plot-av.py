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

        # raw data from packets: [[dts, pts, duration, size], [dts, pts, duration, size], ...]
        self.raw_data_list = []

        # numpy array
        self.npdata = None

    def capture_by_packet(self, packet):
        self.raw_data_list.append(
            [packet.dts, packet.pts, packet.duration, packet.size]
        )

    def finalize(self):
        # construct data array
        # [[dts, pts, duration, size], [dts, pts, duration, size], ...]
        self.npdata = np.array(
            self.raw_data_list,
            dtype=np.float64,
        )
        self.raw_data_list = []  # clean up

        # [[dts, dts, ...], [pts, pts, ...], [duration, duration, ...], [size, size, ...]]
        self.npdata = self.npdata.transpose()

    def dts_in_seconds(self):
        return self.npdata[0] * self.time_base

    def pts_in_seconds(self):
        return self.npdata[1] * self.time_base

    def duration_in_seconds(self):
        return self.npdata[2] * self.time_base

    def size_in_KB(self):
        return self.npdata[3] / 1024.0

    def calc_dts_delta_in_seconds(self):
        dts_delta = self.npdata[0]

        # calculate dts_delta = dts - prev_dts
        dts_delta = np.roll(dts_delta, -1)
        dts_delta[-1] = np.nan  # ignore last value
        dts_delta = dts_delta - self.npdata[0]
        return dts_delta * self.time_base

    def calc_bitrate_in_kbps(self):
        interval = 1 / self.time_base  # 1 second

        data_array = []

        start_ts = self.npdata[0][0]  # first dts
        size = 0

        for d in self.npdata.transpose():  # [[dts,pts,duration,size], ...]
            if d[0] > start_ts + interval:
                data_array.append([start_ts, size])
                start_ts += interval
                size = 0
            size += d[3]

        bitrate = np.array(
            data_array,
            dtype=np.float64,
        ).transpose()
        bitrate[0] = bitrate[0] * self.time_base  # seconds
        bitrate[1] = bitrate[1] * 8 / 1024  # kbps

        return bitrate

    def calc_fps(self):
        interval = 1 / self.time_base  # 1 second

        data_array = []

        start_ts = self.npdata[0][0]  # first dts
        size = 0

        for d in self.npdata.transpose():  # [[dts,pts,duration,size], ...]
            if d[0] > start_ts + interval:
                data_array.append([start_ts, size])
                start_ts += interval
                size = 0
            size += 1

        fps = np.array(
            data_array,
            dtype=np.float64,
        ).transpose()
        fps[0] = fps[0] * self.time_base  # seconds

        return fps


def calc_avsync_in_seconds(v_ts, a_ts, sync_video_against_audio=True):
    # sync method
    base_ts = a_ts if sync_video_against_audio else v_ts
    another_ts = v_ts if sync_video_against_audio else a_ts

    sync_ts = base_ts.copy()

    for index, ts in enumerate(base_ts):
        diff_ts = np.absolute(another_ts - ts)
        min_index = np.argmin(diff_ts)
        sync_ts[index] = diff_ts[min_index]

    return (base_ts, sync_ts)


def plot_av(window_title, v_stream, a_stream):
    # line fmt
    VIDEO_LINE_COLOR = "y"
    VIDEO_LINE_FMT = VIDEO_LINE_COLOR + "^"
    AUDEO_LINE_COLOR = "b"
    AUDIO_LINE_FMT = AUDEO_LINE_COLOR + "+"

    # create axis
    fig, axs = plt.subplots(ncols=3, nrows=3, layout="constrained")
    fig.canvas.manager.set_window_title(window_title)

    # dts
    axs[0, 0].set_title(f"dts")
    axs[0, 0].set_xlabel("packet no.", loc="right")
    axs[0, 0].set_ylabel("dts (s)")
    axs[0, 0].plot(
        v_stream.dts_in_seconds(),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[0, 0].plot(
        a_stream.dts_in_seconds(),
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
        v_stream.pts_in_seconds(),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[0, 1].plot(
        a_stream.pts_in_seconds(),
        AUDIO_LINE_FMT,
        label="audio",
    )
    # axs[0, 1].legend()

    # size
    axs[0, 2].set_title(f"size")
    axs[0, 2].set_xlabel("packet no.", loc="right")
    axs[0, 2].set_ylabel("size (KB)")
    axs[0, 2].plot(
        v_stream.size_in_KB(),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[0, 2].plot(
        a_stream.size_in_KB(),
        AUDIO_LINE_FMT,
        label="audio",
    )
    axs[0, 2].set_ylim(0)

    # avsync
    (base_ts, sync_ts) = calc_avsync_in_seconds(
        v_stream.dts_in_seconds(),
        a_stream.dts_in_seconds(),
    )
    axs[1, 0].set_title(f"av sync")
    axs[1, 0].set_xlabel("time (s)", loc="right")
    axs[1, 0].set_ylabel("diff (s)")
    axs[1, 0].plot(
        base_ts,
        sync_ts,
        "g",
    )
    # axs[1, 0].legend()

    # bitrate
    v_bitrate_array = v_stream.calc_bitrate_in_kbps()
    a_bitrate_array = a_stream.calc_bitrate_in_kbps()
    axs[1, 1].set_title(f"bitrate")
    axs[1, 1].set_xlabel("time (s)", loc="right")
    axs[1, 1].set_ylabel("bitrate (kbps)")
    axs[1, 1].plot(
        v_bitrate_array[0],
        v_bitrate_array[1],
        VIDEO_LINE_COLOR,
        label="video",
    )
    axs[1, 1].plot(
        a_bitrate_array[0],
        a_bitrate_array[1],
        AUDEO_LINE_COLOR,
        label="audio",
    )
    axs[1, 1].set_ylim(0)
    axs[1, 1].legend()

    # fps
    v_fps_array = v_stream.calc_fps()
    a_fps_array = a_stream.calc_fps()
    axs[1, 2].set_title(f"fps")
    axs[1, 2].set_xlabel("time (s)", loc="right")
    axs[1, 2].set_ylabel("fps")
    axs[1, 2].plot(
        v_fps_array[0],
        v_fps_array[1],
        VIDEO_LINE_COLOR,
        label="video",
    )
    axs[1, 2].plot(
        a_fps_array[0],
        a_fps_array[1],
        AUDEO_LINE_COLOR,
        label="audio",
    )
    axs[1, 2].set_ylim(0)
    axs[1, 2].legend()

    # dts delta
    axs[2, 0].set_title(f"dts delta")
    axs[2, 0].set_xlabel("dts (s)", loc="right")
    axs[2, 0].set_ylabel("dts_delta (s)")
    axs[2, 0].plot(
        v_stream.dts_in_seconds(),
        v_stream.calc_dts_delta_in_seconds(),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[2, 0].plot(
        a_stream.dts_in_seconds(),
        a_stream.calc_dts_delta_in_seconds(),
        AUDIO_LINE_FMT,
        label="audio",
    )
    # axs[2, 0].legend()

    # duration
    axs[2, 1].set_title(f"duration")
    axs[2, 1].set_xlabel("dts (s)", loc="right")
    axs[2, 1].set_ylabel("duration (s)")
    # axs[2, 1].yaxis.tick_right()
    # axs[2, 1].yaxis.set_label_position("right")
    axs[2, 1].plot(
        v_stream.dts_in_seconds(),
        v_stream.duration_in_seconds(),
        VIDEO_LINE_FMT,
        label="video",
    )
    axs[2, 1].plot(
        a_stream.dts_in_seconds(),
        a_stream.duration_in_seconds(),
        AUDIO_LINE_FMT,
        label="audio",
    )
    # axs[2, 1].legend()

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

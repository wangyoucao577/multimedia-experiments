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


class AVPlotter:
    # line fmt
    VIDEO_LINE_COLOR = "y"
    VIDEO_LINE_FMT = VIDEO_LINE_COLOR + "^"
    AUDEO_LINE_COLOR = "b"
    AUDIO_LINE_FMT = AUDEO_LINE_COLOR + "+"
    AVSYNC_LINE_COLOR = "g"

    def __init__(self, window_title, v_stream, a_stream):
        self.v_stream = v_stream
        self.a_stream = a_stream
        self.window_title = window_title

    def plot(self):
        # create axis
        fig, axs = plt.subplots(ncols=3, nrows=3, layout="constrained")
        fig.canvas.manager.set_window_title(self.window_title)

        # subplots
        self.plot_dts(axs[0, 0])
        self.plot_pts(axs[0, 1])
        self.plot_size(axs[0, 2])
        self.plot_bitrate(axs[1, 0])
        self.plot_fps(axs[1, 1])
        self.plot_avsync(axs[1, 2])
        self.plot_dts_delta(axs[2, 0])
        self.plot_duration(axs[2, 1])

        # fig.align_labels()
        plt.show()

    def plot_dts(self, ax):
        ax.set_title(f"dts")
        ax.set_xlabel("packet no.", loc="right")
        ax.set_ylabel("dts (s)")
        if self.v_stream:
            ax.plot(
                self.v_stream.dts_in_seconds(),
                self.VIDEO_LINE_FMT,
                label="video",
            )
        if self.a_stream:
            ax.plot(
                self.a_stream.dts_in_seconds(),
                self.AUDIO_LINE_FMT,
                label="audio",
            )
        ax.legend()
        # ax.legend(loc="upper left", bbox_to_anchor=(0.0, 1.02, 0.0, 0.102), ncols=2)

    def plot_pts(self, ax):
        ax.set_title(f"pts")
        ax.set_xlabel("packet no.", loc="right")
        ax.set_ylabel("pts (s)")
        # ax.yaxis.tick_right()
        # ax.yaxis.set_label_position("right")
        if self.v_stream:
            ax.plot(
                self.v_stream.pts_in_seconds(),
                self.VIDEO_LINE_FMT,
                label="video",
            )
        if self.a_stream:
            ax.plot(
                self.a_stream.pts_in_seconds(),
                self.AUDIO_LINE_FMT,
                label="audio",
            )

    def plot_size(self, ax):
        ax.set_title(f"size")
        ax.set_xlabel("packet no.", loc="right")
        ax.set_ylabel("size (KB)")
        if self.v_stream:
            ax.plot(
                self.v_stream.size_in_KB(),
                self.VIDEO_LINE_FMT,
                label="video",
            )
        if self.a_stream:
            ax.plot(
                self.a_stream.size_in_KB(),
                self.AUDIO_LINE_FMT,
                label="audio",
            )
        ax.set_ylim(0)

    def plot_bitrate(self, ax):
        ax.set_title(f"bitrate")
        ax.set_xlabel("time (s)", loc="right")
        ax.set_ylabel("bitrate (kbps)")
        if self.v_stream:
            v_bitrate_array = self.v_stream.calc_bitrate_in_kbps()
            ax.plot(
                v_bitrate_array[0],
                v_bitrate_array[1],
                self.VIDEO_LINE_COLOR,
                label="video",
            )
        if self.a_stream:
            a_bitrate_array = self.a_stream.calc_bitrate_in_kbps()
            ax.plot(
                a_bitrate_array[0],
                a_bitrate_array[1],
                self.AUDEO_LINE_COLOR,
                label="audio",
            )
        ax.set_ylim(0)
        ax.legend()

    def plot_fps(self, ax):
        ax.set_title(f"fps")
        ax.set_xlabel("time (s)", loc="right")
        ax.set_ylabel("fps")
        if self.v_stream:
            v_fps_array = self.v_stream.calc_fps()
            ax.plot(
                v_fps_array[0],
                v_fps_array[1],
                self.VIDEO_LINE_COLOR,
                label="video",
            )
        if self.a_stream:
            a_fps_array = self.a_stream.calc_fps()
            ax.plot(
                a_fps_array[0],
                a_fps_array[1],
                self.AUDEO_LINE_COLOR,
                label="audio",
            )
        ax.set_ylim(0)
        ax.legend()

    def plot_avsync(self, ax):
        ax.set_title(f"av sync")
        ax.set_xlabel("time (s)", loc="right")
        ax.set_ylabel("diff (s)")
        if self.v_stream and self.a_stream:
            (base_ts, sync_ts) = calc_avsync_in_seconds(
                self.v_stream.dts_in_seconds(),
                self.a_stream.dts_in_seconds(),
            )
            ax.plot(
                base_ts,
                sync_ts,
                self.AVSYNC_LINE_COLOR,
            )

    def plot_dts_delta(self, ax):
        ax.set_title(f"dts delta")
        ax.set_xlabel("dts (s)", loc="right")
        ax.set_ylabel("dts_delta (s)")
        if self.v_stream:
            ax.plot(
                self.v_stream.dts_in_seconds(),
                self.v_stream.calc_dts_delta_in_seconds(),
                self.VIDEO_LINE_FMT,
                label="video",
            )
        if self.a_stream:
            ax.plot(
                self.a_stream.dts_in_seconds(),
                self.a_stream.calc_dts_delta_in_seconds(),
                self.AUDIO_LINE_FMT,
                label="audio",
            )

    def plot_duration(self, ax):
        ax.set_title(f"duration")
        ax.set_xlabel("dts (s)", loc="right")
        ax.set_ylabel("duration (s)")
        # ax.yaxis.tick_right()
        # ax.yaxis.set_label_position("right")
        if self.v_stream:
            ax.plot(
                self.v_stream.dts_in_seconds(),
                self.v_stream.duration_in_seconds(),
                self.VIDEO_LINE_FMT,
                label="video",
            )
        if self.a_stream:
            ax.plot(
                self.a_stream.dts_in_seconds(),
                self.a_stream.duration_in_seconds(),
                self.AUDIO_LINE_FMT,
                label="audio",
            )


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
    av_plotter = AVPlotter(os.path.basename(args.input), v_stream, a_stream)
    av_plotter.plot()


if __name__ == "__main__":
    main()


import cv2 as cv
import numpy as np

def main():
    cap = cv.VideoCapture(0)
    if not cap.isOpened():
        print("Cannot open camera")
        exit()
    print(f"camera width={cap.get(cv.CAP_PROP_FRAME_WIDTH)}, height={cap.get(cv.CAP_PROP_FRAME_HEIGHT)}")

    while True:
        # Capture frame-by-frame
        ret, frame = cap.read()
        # if frame is read correctly ret is True
        if not ret:
            print("Can't receive frame (stream end?). Exiting ...")
            break

        # Our operations on the frame come here
        # frame = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)

        # Display the resulting frame
        cv.imshow('frame', frame)
        if cv.waitKey(1) == ord('q'):
            break

    # When everything done, release the capture
    cap.release()
    cv.destroyAllWindows()


if __name__ == "__main__":
    main()

import cv2 as cv
import numpy as np


def main():
    img = cv.imread("1.jpg")

    # Covert BGR to HSV
    hsv = cv.cvtColor(img, cv.COLOR_BGR2HSV)

    # define range of blue color in HSV
    lower_blue = np.array([110, 50, 50])
    upper_blue = np.array([130, 255, 255])

    # Threshold the HSV image to get only blue colors
    mask = cv.inRange(hsv, lower_blue, upper_blue)

    # Bitwise-AND mask and original image
    res = cv.bitwise_and(img, img, mask=mask)

    cv.imshow("frame", img)
    cv.imshow("mask", mask)
    cv.imshow("res", res)
    k = cv.waitKey(0) & 0xFF

    cv.destroyAllWindows()


if __name__ == "__main__":
    main()

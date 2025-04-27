import cv2
import numpy as np

# 读图、SIFT、匹配、ratio test
img1 = cv2.imread('./computer1.jpg')
img2 = cv2.imread('./computer2.jpg')

gray1 = cv2.cvtColor(img1,cv2.COLOR_BGR2GRAY)
gray2 = cv2.cvtColor(img2,cv2.COLOR_BGR2GRAY)

sift = cv2.SIFT_create()

kp1,des1 = sift.detectAndCompute(gray1,None)
kp2,des2 = sift.detectAndCompute(gray2,None)

bf = cv2.BFMatcher()

match = bf.knnMatch(des1,des2,k=2)

good = []

for m,n in match:
    if m.distance < 0.75 * n.distance:
        good.append(m)
if len(good) < 4:
    exit()


src_point = np.float32([kp1[m.queryIdx].pt for m in good]).reshape(-1,1,2)
dst_point = np.float32([kp2[m.trainIdx].pt for m in good]).reshape(-1,1,2)
H,_ = cv2.findHomography(src_point,dst_point,cv2.RANSAC,5)

h1,w1 = img1.shape[:2]
h2,w2 = img2.shape[:2]
img1_pts = np.float32([[0,0],[0,h1-1],[w1-1,0],[w1-1,h1-1]]).reshape(-1,1,2)
img2_pts = np.float32([[0,0],[0,h2-1],[w2-1,0],[w2-1,h2-1]]).reshape(-1,1,2)
img1_transform = cv2.perspectiveTransform(img1_pts,H)
all_pts = np.concatenate((img2_pts,img1_transform),axis=0)
x_min,y_min = np.int32(all_pts.min(axis=0).ravel()-1)
x_max,y_max = np.int32(all_pts.max(axis=0).ravel()+1)


canvas_size = (x_max - x_min,y_max - y_min)

offset = np.array([[1,0,-x_min],[0,1,-y_min],[0,0,1]])

result = cv2.warpPerspective(img1,offset.dot(H),canvas_size)
result[-y_min:-y_min+h2,-x_min+0:-x_min+w2] = img2

cv2.namedWindow('window',cv2.WINDOW_NORMAL)
cv2.resizeWindow('window',result.shape[1],result.shape[0])
cv2.imshow('window',result)
cv2.waitKey(0)
cv2.destroyAllWindows()



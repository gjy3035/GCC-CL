from PIL import Image
import numpy as np
import matplotlib.pyplot as plt
import time
import re
import sys
import os
import os.path as path
import json

headJson = []

class pedestrian:
    def __init__(self, info):
        self.id = int(info[0])
        boneLoc = map(lambda x: re.findall(r" x=(.*?) y=(.*?) />", x)[0], info[1].strip().split('\n\t'))
        bones = boneLoc
        self.bones = np.array(list(bones), dtype='float32')

def render(stencil, origin):
    x, y, z = origin.shape
    nshape = np.array([255, 255, 0], dtype='uint8')
    psize = 3
    top, bottom = max(0, stencil[0] - psize), min(x, stencil[0] + psize)
    left, right = max(0, stencil[1] - psize), min(y, stencil[1] + psize)
    # print(stencil, top, bottom, left, right)
    mark = np.ones((bottom - top, right - left), dtype='uint8')
    mark = np.pad(mark, ((top, x - bottom), (left, y - right)), 'constant')
    for i in range(z):
        origin[:, :, i] = np.where(mark == 1, nshape[i], origin[:, :, i])
    return origin

def multiRender(bmp, p, stencil):
    boneLoc = np.dot(p.bones, np.array([[0, 1920], [1080, 0]])).round().astype('int')
    headLoc = (boneLoc[0] - 1).tolist()
    x, y = headLoc
    # print(p.bones, ':', headLoc)
    if x >= 0 and y >= 0 and stencil[x, y]:
        bmp = render(headLoc, bmp)
        headJson.append(headLoc)
    return bmp

def runRender(fold, peds, rendfunc, bmpName='result_1'):
    bmpArray = np.array(Image.open(path.join(fold, 'combination.bmp')))
    stencil = np.fromfile(path.join(fold, 'combination.npy'), 'uint8')
    stencil = stencil.reshape(1080, 1920)
    for p in peds:
        if p.bones.shape[0] == 0:
            continue
        # print("No.{} Ok.".format(i))
        bmpArray = rendfunc(bmpArray, p, stencil)
    bmp = Image.fromarray(bmpArray)
    bmp.save(path.join(fold, bmpName+'.jpg'))

def main(fold):
    peds = []
    with open(path.join(fold, 'pedInfo.xml')) as f:
        info = re.findall(
            r'<ped id=(\d+?)>([\s\S]*?)</ped>', f.read())
        peds = list(map(pedestrian, info))
    runRender(fold, peds, multiRender, bmpName='result')
    with open(path.join(fold, 'pedInfo.json'), 'w+') as f:
        global headJson
        json.dump(headJson, f)
        headJson = []


if __name__ == '__main__':
    main(sys.argv[1])

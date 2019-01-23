# -*- coding: utf-8 -*-

from PIL import Image
import numpy as np
# import matplotlib.pyplot as plt
import time
import re
import os
import sys
import os.path as path
from functools import reduce


getImgArray = lambda img: np.array(Image.open(img))

def getCurve(name):
    stencil = np.fromfile(name, 'uint8')
    stencil = stencil.reshape(1080, 1920)
    stencil = np.where(np.isin(stencil, (1, 17)), 1, 0).astype('uint8')# OUTDOOR
    # stencil = np.where(np.isin(stencil, (9, 25)), 1, 0).astype('uint8')# INDOOR
    return stencil

def getorder(fileNames):
    filterFunc = lambda img: '.raw' in img
    imgFileNames = filter(filterFunc, fileNames)
    order = sorted(int(re.findall(r'(\d+?).raw', imgFileName)[0]) 
                    for imgFileName in imgFileNames)
    return order

def backMask(fold, order):
    mask = np.full((1080, 1920), len(order), dtype='uint8')
    rgbDis = np.zeros((1080, 1920), dtype='int')
    imgTemp = path.join(fold, 'part_{}_{}.bmp')
    for o in order:
        # cutout background
        back = getImgArray(imgTemp.format(o, 0)).astype('int')
        fore = getImgArray(imgTemp.format(o, 1)).astype('int')
        diff = np.sum((fore - back) ** 2, axis=2)
        mask = np.where(diff > rgbDis, o, mask)
        rgbDis = np.where(diff > rgbDis, diff, rgbDis)
    return mask

def combine(fold):
    # get image order
    order = getorder(os.listdir(fold))
    # get background
    mask= backMask(fold, order)
    curveTemp = path.join(fold, 'part_{}.raw')
    for o in order:
        curve = getCurve(curveTemp.format(o))
        mask = np.where(curve==1, o, mask)
    mask = np.concatenate([mask[:,:,np.newaxis]]*3, axis=2)

    imgTemp = path.join(fold, 'part_{}_{}.bmp')
    fimg = getImgArray(imgTemp.format(0, 1))
    for o in order[1:]:
        img = getImgArray(imgTemp.format(o, 1))
        fimg = np.where(mask==o, img, fimg)
    fimg = Image.fromarray(fimg)
    fimg.save(path.join(fold, 'combination.png'))
    fimg.save(path.join(fold, 'combination.jpg'))
    fimg.save(path.join(fold, 'combination.bmp'))

    raw = getCurve(curveTemp.format(order[0]))
    for o in order[1:]:
        part = getCurve(curveTemp.format(o))
        raw = np.where(part == 1, part, raw)
    raw.reshape(-1).tofile(path.join(fold, 'combination.npy'))


def renderCurve(fold, No):
    stencil = getCurve(path.join(fold, "base_{}.raw".format(No)))
    stencil = stencil[:, :, np.newaxis]
    stencil = np.concatenate([stencil] * 3, axis=2)
    bmp = np.array(Image.open(path.join(fold, "base_{}.bmp".format(No))))
    combImg = np.where(stencil == 1, bmp, 0)
    bmp = Image.fromarray(combImg)
    bmp.save(path.join(fold, 'combination_{}.png'.format(No)))
    bmp.save(path.join(fold, 'combination_{}.jpg'.format(No)))

if __name__ == '__main__':
    # combine(sys.argv[1])
    combine('source/part_11_2/1534540881')

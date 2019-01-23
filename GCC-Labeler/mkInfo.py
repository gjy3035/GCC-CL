# -*- coding: utf-8 -*-

import scipy.io as scio
import numpy as np
import json
import re
import os

def getInfo(fold, subfold):
    global weather, timeInfo, roi, camera, headMatrix
    with open(os.path.join(subfold, 'pedInfo.xml')) as f:
        txt = f.read()
        weather = re.findall(r'<weather>(.*?)</weather>', txt)[0]
        timeInfo = re.findall(r'time hour=(\d*?)minutes=(\d*?) />', txt)[0]
        timeInfo = list(map(int, timeInfo))
    with open(os.path.join(fold, 'areaInfo.log')) as f:
        txt = f.read().split()
        number, location = int(txt[0]), list(map(float, txt[1:]))
        roix, roiy = location[:number], location[number:]
        roi = list(zip(roix, roiy))
    with open(os.path.join(fold, 'eyeInfo.log')) as f:
        txt = f.read().split('\n')
        camera = {
            'location': list(map(float, txt[0].split())),
            'rotation': list(map(float, txt[1].split())),
            'fov': float(txt[2])
        }
    with open(os.path.join(subfold, 'pedInfo.json')) as f:
        headMatrix = json.load(f)




def mkMat(savePath):
    # head location
    location = np.array(headMatrix, dtype='int')
    number = np.array([[len(headMatrix)]], dtype='int')
    image_info = (location, number)
    matMatrix = np.array(image_info, dtype=np.dtype([('location', 'O'), ('number', 'O')]))
    # weatehr
    matWeather = np.array([[weather]])
    # time infomation
    matTime = np.array([timeInfo], dtype='uint8')
    # roi
    matRoi = np.array(roi)
    # camera
    matCamera = np.array([[(
        np.array([camera['location']]), 
        np.array([camera['rotation']]), 
        np.array([[camera['fov']]])
        )]],
        dtype = ([('location', 'O'), ('rotation', 'O'), ('fov', 'O')])
    )
    scio.savemat(savePath, {'image_info': matMatrix, 
        'weather_info': matWeather,
        'time_info': matTime,
        'roi_info': matRoi,
        'camera_info': matCamera
    })

def mkJson(savePath):
    jsdict = {
        'image_info': headMatrix,
        'weather': weather,
        'timeInfo': timeInfo,
        'roi': roi,
        'camera': camera
    }
    with open(savePath, 'w+') as f:
        json.dump(jsdict, f)

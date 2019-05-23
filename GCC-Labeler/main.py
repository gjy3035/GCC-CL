# -*- coding: utf-8 -*-

import sys, time, os, shutil
import os.path as path
import re

from combine import combine
from getHead import main as getHeadMain
from mkInfo import getInfo, mkJson, mkMat

def deleteFile(fold):
    for fi in os.listdir(fold):
        if 'part' in fi or fi == 'pedInfo.xml':
            continue
        os.remove(path.join(fold, fi))

def mkdir(goal_dir):
    if not path.exists(goal_dir):
        os.makedirs(goal_dir)

if __name__ == '__main__':
    source_dir = 'source'
    target_dir = 'target'
    mkdir(target_dir)
    for part_dir in os.listdir(source_dir):
        if re.match(r'part_\d+_\d', part_dir) == None:
            continue
        print('---', part_dir, '---')
        start, foldNo = time.time(), 1

        goal_dir = path.join(target_dir, 'scene' + part_dir[4:])
        mkdir(goal_dir)
            
        
        subFolds = ('pngs', 'jpgs', 'jsons', 'mats', 'vis', 'segs')
        for subFold in subFolds:
            mkdir(path.join(goal_dir, subFold))

        for foldName in os.listdir(os.path.join(source_dir, part_dir)):
            fold = path.join(source_dir, part_dir, foldName)
            if path.isdir(fold) and "result" not in fold:
                try:
                    combine(fold)
                    getHeadMain(fold)

                    # move image
                    shutil.move(path.join(fold, 'combination.png'), path.join(goal_dir, 'pngs', foldName + '.png'))
                    shutil.move(path.join(fold, 'combination.jpg'), path.join(goal_dir, 'jpgs', foldName + '.jpg'))

                    # write json and mat
                    getInfo(os.path.join(source_dir, part_dir), fold)
                    mkJson(path.join(goal_dir, 'jsons', foldName + '.json'))
                    mkMat(path.join(goal_dir, 'mats', foldName + '.mat'))
                    
                    # move seg
                    shutil.move(path.join(fold, 'combination.npy'), path.join(goal_dir, 'segs', foldName + '.raw'))
                    
                    # move vis
                    shutil.move(path.join(fold, 'result.jpg'), path.join(goal_dir, 'vis', foldName + '.jpg'))
                except :
                    with open('error.log', 'a+') as f:
                        print('catch error in fold [' + fold + ']', file=f)
                        print('[fold No.{}]'.format(foldNo), foldName, 'error.')
                else:
                    print('[fold No.{}]'.format(foldNo), foldName, 'done.')
                    deleteFile(fold)
                finally:
                    foldNo += 1
        end = time.time()
        print('cost time:', end - start)
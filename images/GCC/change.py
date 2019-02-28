# -*- coding: utf-8 -*-

import numpy as np
from PIL import Image
import os

fname = lambda x: os.path.join('..', x)

for png in os.listdir('..'):
    if png[-3:] != 'png':
        continue
    os.remove(fname(png))

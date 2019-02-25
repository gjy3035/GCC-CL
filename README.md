# GCC-CL

GCC-CL(**GCC** dataset **C**ollector and **L**abeler) is a tool for creating synethic crowd images dataset. It consist two parts, collector and labeler, which are used to generate image information and generate input/output for crowd count model respectively.

**Attention!!!** All mods must be used in the offline version of GTA V. 

## GCC-Collector

GCC-Collector is a tool to generate crowd image and head point information. It is a custom plugin running along with GTAV, written in **C++**. with the exception of basic windows C++ API, It also need:
- [ScriptHookV](http://www.dev-c.com/gtav/scripthookv/)
- [DirectXTK](https://github.com/Microsoft/DirectXTK)
- [minhook](https://github.com/TsudaKageyu/minhook)
- [eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)

### Compilation

1. navigate to GCC-Collector directory, use Visual Studio to open the project `GCC-Collector.sln`;
2. open the project(GCC-Collector) property pages, `General/Windos SDK Version`, **select the latest win10 SDK** in your computer
3. using `NuGet` install **minihook** and **eigen**
4. in `GCC-Collector/deps`，The compiled DirectXTK and the lastest Script Hook V have been added into project.
5. make sure Configution is **release**, Platform is **x64**, and then `ctrl + shift + B` to compile the project, The `GCC-Collector.asi` will be generated.

### Installation

1. Following [Script Hook V](http://www.dev-c.com/gtav/scripthookv/) installation steps, copy `ScriptHookV.dll`, `dinput8.dll` and compiled `GCC-Collector.asi` to the game's main folder(where GTA5.exe is located);
2. Use [this mod](https://www.gta5-mods.com/misc/no-chromatic-aberration-lens-distortion-1-41) to avoid chromatic aberration and lens distortion;
3. Compile or directly use the compiled plugin-in `unlimitedLife.asi`, `noVehicle.asi`. Just copy these two asi to the games's main folder.
> - `unlimitedLife.asi` make sure the player undead. 
> - `noVehicle.asi` is optional. when you create some crowd image in street, you would not want some vehicle come into screen and cause an accident. This plugin-in avoid this things.

### How to run

To create a series of crowd images in one scene, you should do these things:
1. create a directory named `data` in the game's main folder.
2. control your role in GTA5 to go to a place, which will be the background of the crowd images.
3. press `F9` to start a scene script;
4. press `F10` to adjust the camera. use `W, A, S, D` to move the camera forward, left, backward and right, mouse button to move up or down, `shift/ctrl` to speed up or slow down, and `+/-` to adjust the field of view. when the camera is ok, press `F10` again to stop adjust camera and record camera location information automatically.
5. press `F11` to define the area where the crowd will be generated. It would be better if you create a convex area. press `I` to recorded one point of the area, and `F10` to toggle to the camera location you defined and press `F10` again you will back tou the role whole you control to define the area. If you want to reset some point of the area , just press `Tab` to the point and reset it. In the end , press `F11` again to end define the area. Following the guide to readjust the camera, and another three camera lacations(from four different angle).
6. At last, you should make sure how many pedestrians will be generated in the crowd. Press `F12` and follow the guide in the game, you will know how to do it.
7. Now one scene has been created and saved, press `F5` to go back to original state, and do above steps(except create 'data' fold) again to recorded another background and relevant information.

After a series of background have been created. restart the game, after the opening animation, press `L` to launch the GCC-Collector to generate crowd with recorded information.

## GCC-Labeler

GCC-Labeler is written in Python3. It need thesepython library:
- scipy
- numpy
- matplotlib
- PIL

All these could be installed by `pip`.

`main.py` is the entrance of the project. The line 22 and 23
```
source_dir = 'source'
target_dir = 'target'
```
defined the data source and target directory name. you can change it to an directory you like. just run `python main.py`, the final crowd count images and labels will be generated in `target_dir`.

For example, the source directory(the data folder we created in GCC-Collector) like this:
```
source
`-- part_11_2
    |-- 1534540881
    |   |-- part_0.raw
    |   |-- part_0_0.bmp
    |   |-- part_0_1.bmp
    |   |-- part_180.raw
    |   |-- part_180_0.bmp
    |   |-- part_180_1.bmp
    |   |-- part_360.raw
    |   |-- part_360_0.bmp
    |   |-- part_360_1.bmp
    |   `-- pedInfo.xml
    |-- 1534575913
    |   |-- part_0.raw
    |   |-- part_0_0.bmp
    |   |-- part_0_1.bmp
    |   |-- part_180.raw
    |   |-- part_180_0.bmp
    |   |-- part_180_1.bmp
    |   |-- part_360.raw
    |   |-- part_360_0.bmp
    |   |-- part_360_1.bmp
    |   `-- pedInfo.xml
    |-- Zheight.log
    |-- areaInfo.log
    |-- eyeInfo.log
    `-- levelInfo.log

3 directories, 24 files
```
After we run the `main.py`, we can get the target directory like this:
```
target/
`-- scene_11_2
    |-- jpgs
    |   |-- 1534540881.jpg
    |   `-- 1534575913.jpg
    |-- jsons
    |   |-- 1534540881.json
    |   `-- 1534575913.json
    |-- mats
    |   |-- 1534540881.mat
    |   `-- 1534575913.mat
    |-- pngs
    |   |-- 1534540881.png
    |   `-- 1534575913.png
    |-- segs
    |   |-- 1534540881.raw
    |   `-- 1534575913.raw
    `-- vis
        |-- 1534540881.jpg
        `-- 1534575913.jpg

7 directories, 12 files
```

# GCC-CL

GCC-CL(**GCC** dataset **C**ollector and **L**abeler) is a tool for generating synethic crowd image datasets. It consists of two parts, collector and labeler.  The former is used to generate crowd image information; the latter produce input and output file through those information for crowd count models. 

## GCC-Collector

GCC-Collector is a tool to generate crowd image and head points in GTA V. It is a custom plugin running along with GTAV, written in **C++**.

**Attention!!!** All mods must be used in the offline version of GTA V.

### Request
- [ScriptHookV](http://www.dev-c.com/gtav/scripthookv/)
- [DirectXTK](https://github.com/Microsoft/DirectXTK)
- [minhook](https://github.com/TsudaKageyu/minhook)
- [eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)

### Compilation

1. Navigate to GCC-Collector directory, use Visual Studio to open the project `GCC-Collector.sln`.
2. Open the project (GCC-Collector) property pages, `General/Windos SDK Version`, select **the latest win10 SDK** existed in your computer.
3. Install `minhook` and `eigen` using  **NuGet**.
4. Make sure Configution is **release**, Platform is **x64**, and then use hot key `ctrl + shift + B` to compile the project. As a result, asi file `GCC-Collector.asi` will be generated in */GCC-CL/GCC-Collector/x64/Release*

### Installation

1. Following [Script Hook V](http://www.dev-c.com/gtav/scripthookv/) installation steps, copy `ScriptHookV.dll`, `dinput8.dll` and compiled `GCC-Collector.asi` to the game's root folder (where GTA5.exe is located);
2. Use [this mod(No Chromatic aberration & Lens distortion)](https://www.gta5-mods.com/misc/no-chromatic-aberration-lens-distortion-1-41) to avoid chromatic aberration and lens distortion;
3. Compile or directly use the compiled plugin-in `unlimitedLife.asi`, `noVehicle.asi`. Just copy these two asi to the games's root folder.
> - `unlimitedLife.asi` makes sure the player undead. 
> - `noVehicle.asi` is optional. When you are creating some crowd images in the street, this plugin-in helps you avoid accidents caused by vehicles entering the scene.

### How to work

To create a series of crowd images in one scene, You should follow the steps below:
1. Create a directory named `data` in the game's root folder.
2. Open GTA V in the **offline version**.
2. Control your role in GTAV go to the scene where you want use it as the background of the crowd images which would be generated.
3. Press `F9` to start a scene script;
4. Press `F10` to adjust the camera. use `W, A, S, D` to move the camera forward, left, backward and right, mouse button(left or right button both works) to move up or down, `shift/ctrl` to speed up or slow down, and `+/-` to adjust the field of view. When the camera has the desired angle, press `F10` again to stop adjust camera and record camera location information automatically.
5. Press `F11` to start drawing the crowd generation area. Use the method of connecting multiple points to form a polygon by moving your game character in the scene. Press `I` to set a polygon vertex at current position. Press `F11` again to end the drawing. The program automatically connects the points in a sequence set by the user. In the process of setting the vertices, you can press `Tab` to move the focus to a specific vertex, and then you can reset that point. During setting the vertices, you can press `F10` to enter the camera view to observe the vertices or modify the camera settings. Press `F10` again to return to set vertices. (Note that it will be better to set a convex polygon area, do not let the sides of the polygon cross.) Following the guide to readjust the camera, and other three cameras (from four different angle). 
6. At last, you should confirm how many pedestrians you want to generate in the scene. Press `F12` and follow the guide in the game, you will know how to do it.
7. Now one scene has been created and saved, press `F5` to go back to original state, and do above steps(except step 1) again to recorded another scene.

After a series of scene have been created. restart the game, after the opening animation, press `L` to launch the GCC-Collector to generate crowd with recorded information.

## GCC-Labeler

GCC-Labeler is written in Python3. It needs the following Python libraries:
- scipy
- numpy
- matplotlib
- PIL

You can install these libraries using `pip`.

`main.py` is the entrance of the project. Line 22 and 23 define the source path and the target path.
``` python
source_dir = 'source'
target_dir = 'target'
```
Source path is the path where the file you generated using GCC-Collector is located. The target path is used to specify where to store the final produced image files and the annotation files. Change two paths according to your actual situation. Just run `python main.py`, the final crowd count images and labels will be generated in `target_dir`.

For example, the source directory(the data folder we created in GCC-Collector) like this:
```
source/
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

## Acknowledgments

Some code borrows from [gtav-mod-scene-director](https://github.com/elsewhat/gtav-mod-scene-director) and [GTAVisionExport](https://github.com/umautobots/GTAVisionExport). The former gave us so many examples for how to use these functions in Script Hook V. The latter teached us a way to extract crowd mask.
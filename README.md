# microramfs-creator
Creates microramfs init array from folder.
# microramfs
microramfs is virtual filesystem used in GumOS https://github.com/Andrewerr/GumOS <br>
It loads from static array. To get this initializtion array you need to have a folder with data you would like to store in microramfs (system folders like ```/dev```, ```/var/lock```, ```/etc``` should be included as they not created on initialization)
# Compilation
```gcc main.c -o init_microramfs```
# Set permissions to exexcute
``` chmod +x init_microramfs ```
# Usage
``` ./init_microramfs <DIRNAME>``` where ```DIRNAME``` is path to folder where microramfs data is stored. This will convert all files and directories in ```DIRNAME``` into microramfs initialization array.
# Output
On output you will get byte array and it's size. 
# Known issues 
There is a comma before closing curly bracket. Issue #1

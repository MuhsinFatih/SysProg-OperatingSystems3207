# Filesystem
In this project, I implemented a linux-like full binary filesystem. I used inodes and tried to implemented the filesystem in a maintainable fashion, meaning all the filesystem properties are variables that can be changed easily, like the block size, inode size in terms of blocks etc. I also partially implemented some features so that more advanced implementations can be added without too much change on the code. For example, volumes with varying sizes can be added via partitioning to the filesystem, features like journaling symlinking can be added without much effort (If I had more time before finals I was going to implement them as well)

## Introduction
mufafs is a filesystem that makes use of inodes and indirect addressing. Basic functionalities are: 
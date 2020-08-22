#!/bin/bash

export mesh=$1

../../bin/KoebeIteration -circular_slit_map ${mesh}_0.m ${mesh}_1.m ../../textures/checker_1k.bmp
../../bin/KoebeIteration -fill_hole ${mesh}_1.m ${mesh}_1_filled.m
../../bin/KoebeIteration -view ${mesh}_1_filled.m ../../textures/checker_1k.bmp

../../bin/KoebeIteration -circular_slit_map ${mesh}_1_filled.m ${mesh}_2.m ../../textures/checker_1k.bmp
../../bin/KoebeIteration -fill_hole ${mesh}_2.m ${mesh}_2_filled.m
../../bin/KoebeIteration -view ${mesh}_2_filled.m ../../textures/checker_1k.bmp

../../bin/KoebeIteration -circular_slit_map ${mesh}_2_filled.m ${mesh}_3.m ../../textures/checker_1k.bmp
../../bin/KoebeIteration -fill_hole ${mesh}_3.m ${mesh}_3_filled.m
../../bin/KoebeIteration -view ${mesh}_3_filled.m ../../textures/checker_1k.bmp

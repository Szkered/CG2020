set mesh=%1

..\..\bin\KoebeIteration.exe -punch_hole step_0.m %mesh%_3_filled.m punch_1.m 1
..\..\bin\KoebeIteration.exe -circular_slit_map punch_1.m punch_1_cf.m ..\..\textures\checker_1k.bmp
..\..\bin\KoebeIteration.exe -fill_hole punch_1_cf.m punch_1_filled.m
..\..\bin\KoebeIteration.exe -view punch_1_filled.m ..\..\textures\checker_1k.bmp

..\..\bin\KoebeIteration.exe -punch_hole step_0.m punch_1_filled.m punch_2.m 2
..\..\bin\KoebeIteration.exe -circular_slit_map punch_2.m punch_2_cf.m ..\..\textures\checker_1k.bmp
..\..\bin\KoebeIteration.exe -fill_hole punch_2_cf.m punch_2_filled.m
..\..\bin\KoebeIteration.exe -view punch_2_filled.m ..\..\textures\checker_1k.bmp

..\..\bin\KoebeIteration.exe -punch_hole step_0.m punch_2_filled.m punch_3.m 3
..\..\bin\KoebeIteration.exe -circular_slit_map punch_3.m punch_3_cf.m ..\..\textures\checker_1k.bmp
..\..\bin\KoebeIteration.exe -fill_hole punch_3_cf.m punch_3_filled.m
..\..\bin\KoebeIteration.exe -view punch_3_filled.m ..\..\textures\checker_1k.bmp

# Files used in the study: 
Controls on Asymmetric Continental Crustal Thinning to Symmetric Mantle Exhumation at Magma-poor Rifted Margins: Processes constrained 
by the West Iberia-Newfoundland Conjugate Margins.

Yuan Wang, Zhonglan Liu, W Roger Buck, Xuan-Long Shan, Dian-Ju Wang, Wei-Min Li, Wen-Liang Xu


Model runs in the study used dealii 9.2.0.

The software version used for this study is found at: 

ASPECT 2.3

        https://github.com/Djneu/aspect/tree/fault_analysis
	
FastScape commit 18f2588

(elimated)

        https://github.com/fastscape-lem/fastscapelib-fortran 
	
(backup)

        https://github.com/Jlu-geodynamic/fastscape_backup_commit18f2588

# Additional ASPECT plugins statement:

1."comp" and "rift" were modified from the plugins used in the study: 


Evolution of rift systems and their fault networks in response to surface processes

Derek Neuharth, Sascha Brune, Thilo Wrona, Anne Glerum, Jean Braun, Xiaoping Yuan 


We change some initial setting to fit the grain size evolution.


2."grain" were modified from the sorce code. We added grain size evolution in the ViscoPlastic model.


To install ASPECT with FastScape

1.Get ASPECT and FastScape from the sites above.

2.Create a build directory for fastscape and compile it with an added flag for creating a shared library.
                cmake -DBUILD_FASTSCAPELIB_SHARED=ON /path/to/fastscapemake
	make

3.Compile ASPECT with a flag FASTSCAPE_DIR pointing to the fastscape build folder with the shared library
	cmake -DFASTSCAPE_DIR=/path/to/fastscape/build -DDEAL_II_DIR=/path/to/dealii -DCMAKE_BUILD_TYPE=Release /path/to/aspect
	make


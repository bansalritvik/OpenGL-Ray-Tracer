An .exe for my project can be found at 'Release/Assignment2.exe'

The solution and project for this assignment were built for VS 2010.

I tested this zip using a VS 2008 VCL reservation, though, since there aren't any VS 2010 VCL machines.

If you cannot open the .sln file in VS 2010, you can copy my source file from 'Assignment1/assignment1.c'
into a project with opengl & glut linked.

My program expects the input obj file to be located in the same directory as the .exe and named 'input.obj'.

For some reason, my normal interpolation is weird sometimes. If you are unable to see it with
	your models, please let me know and I can provide some models where it is visible.

If VS2010 is not installed, you may get an error stating that 'msvcr100.dll' is missing, so I have included
	that dll in 'Release/'. Simply remove the '.remove' extension from that file.

Extra Credit:
-Press 'l' to toggle the light from (0,5,0) (0,5,-5) for better specular effects on some
	objects (starts at (0,0,0))
-Press 's' to enable shadow detection (off by default)
-Press 'r' to enable coherent reflection (off by default)
-Press 'f' to enable refraction (off by default)
-Press 'u' to "unitize" a model (move it to the origin and scale it to fit in a
	(1,1,1) cube around the origin - cannot be toggled; application must be restarted to undo)
Note: coherent reflection and refraction work to an extend, but I had to make some
	assumptions to make them work. They both are added to the local lighting
	calculation after being multiple by .2 to cut back on their contribution.
	For refraction, I assumed that the initial material is air (n=1) and the new
	material is glass (n=1.52).
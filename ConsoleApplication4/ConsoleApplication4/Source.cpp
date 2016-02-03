#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <string.h>
#include <assert.h>
#include <CL/cl.h>

#define MAX_SOURCE 0x100000

	cl_platform_id platformid=NULL;
	cl_device_id deviceid=NULL;
	cl_uint ret_numdevices,ret_numplatforms;
	cl_int ret;
	cl_context context;
	cl_command_queue cmdq;
	cl_program program;
	cl_kernel kernel;

	cl_mem xiobj,yiobj,ziobj;
	cl_mem xoobj,yoobj,zoobj;

	float *xi=NULL, *yi=NULL, *zi=NULL;
	int inputtriangles=4;
	int inputvertices=3*inputtriangles;

	float *xo=NULL, *yo=NULL, *zo=NULL;
	int outputtriangles=4*inputtriangles;
	int outputvertices=3*outputtriangles;

	size_t global_size_item[1] = {inputtriangles};	
	size_t local_size_item[1] = {1};


//OpenGL STUFF BEGINS
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_util.h"
#include "math_3d.h"
#include "pipeline.h"

GLuint VBO;
//GLuint glColor;
GLuint gWorldLocation;

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

//OpenGL Functions Begin
static void CreateVertexBuffer(float x1, float y1, float z1,float x2, float y2, float z2,float x3, float y3, float z3);

struct vertex0
{
	float x,y,z;

	vertex0()
	{
		x=-1.0f;
		y=-1.0f;
		z=0.5773f;
	}	
}v0;

struct vertex1
{
	float x,y,z;

	vertex1()
	{
		x=0.0f;
		y=-1.0f;
		z=-1.15475f;
	}	
}v1;

struct vertex2
{
	float x,y,z;

	vertex2()
	{
		x=1.0f;
		y=-1.0f;
		z=0.5773f;
	}	
}v2;

struct vertex3
{
	float x,y,z;

	vertex3()
	{
		x=0.0f;
		y=1.0f;
		z=0.0f;
	}	
}v3;

struct Vertex
{
    Vector3f m_pos;
    Vector4f m_col;

    Vertex() {}

    Vertex(Vector3f pos, Vector4f col)
    {
        m_pos = pos;
        m_col = col;
    }
};

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	static float Scale = 0.0f;
    Scale += 0.02f;

	Pipeline p;
	p.WorldPos(0.0f, 0.0f, 5.0f);
	p.Rotate(0.0f, Scale, 0.0f);
    p.SetPerspectiveProj(30.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 100.0f);
	glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());

		//Drawing on Screen
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
		glDrawArrays(GL_TRIANGLES, 0, outputvertices);				//Last parameter is TOTAL number of Vertices to be drawn

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

    glutSwapBuffers();
}

static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
	glutIdleFunc(RenderSceneCB);
}

static void CreateVertexBuffer()
{
	int n=outputvertices;

	float r,g,b;
	Vertex *Vertices = new Vertex[n];
	
	for(int i=0;i<n;i++)
	{
		switch(i%3)
		{
			case 0:	r=1.0; g=0.0;	b=0.0f;	break;
			case 1: r=0.0; g=1.0;	b=0.0f;	break;
			case 2: r=0.0; g=0.0;	b=1.0f;	break;
		}
		Vertices[i] = Vertex(Vector3f(xo[i]-0.1*sin(xo[i]*4*3.141),yo[i],zo[i]), Vector4f(r,g,b,1.0));
	}

 	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*n, Vertices, GL_STATIC_DRAW);

	free(xi);	free(yi);	free(zi);
	free(xo);	free(yo);	free(zo);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0]= strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj);
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }
    
    string vs, fs;

    if (!ReadFile(pVSFileName, vs)) {
        exit(1);
    };

    if (!ReadFile(pFSFileName, fs)) {
        exit(1);
    };

    AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
    AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
	}

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);

	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
    assert(gWorldLocation != 0xFFFFFFFF);
}

//OpenGL Functions ENDS

void KernelExecution()
{
	xiobj=clCreateBuffer(context,CL_MEM_READ_WRITE,sizeof(float)*inputvertices,NULL,&ret);
	yiobj=clCreateBuffer(context,CL_MEM_READ_WRITE,sizeof(float)*inputvertices,NULL,&ret);
	ziobj=clCreateBuffer(context,CL_MEM_READ_WRITE,sizeof(float)*inputvertices,NULL,&ret);

	xoobj=clCreateBuffer(context,CL_MEM_READ_WRITE,sizeof(float)*outputvertices,NULL,&ret);
	yoobj=clCreateBuffer(context,CL_MEM_READ_WRITE,sizeof(float)*outputvertices,NULL,&ret);
	zoobj=clCreateBuffer(context,CL_MEM_READ_WRITE,sizeof(float)*outputvertices,NULL,&ret);

	ret=clEnqueueWriteBuffer(cmdq,xiobj,CL_TRUE,0,sizeof(float)*inputvertices,xi,0,NULL,NULL);
	ret=clEnqueueWriteBuffer(cmdq,yiobj,CL_TRUE,0,sizeof(float)*inputvertices,yi,0,NULL,NULL);
	ret=clEnqueueWriteBuffer(cmdq,ziobj,CL_TRUE,0,sizeof(float)*inputvertices,zi,0,NULL,NULL);

	ret=clSetKernelArg(kernel,0,sizeof(cl_mem),(void *)&xiobj);
	ret=clSetKernelArg(kernel,1,sizeof(cl_mem),(void *)&yiobj);
	ret=clSetKernelArg(kernel,2,sizeof(cl_mem),(void *)&ziobj);
	ret=clSetKernelArg(kernel,3,sizeof(cl_mem),(void *)&xoobj);
	ret=clSetKernelArg(kernel,4,sizeof(cl_mem),(void *)&yoobj);
	ret=clSetKernelArg(kernel,5,sizeof(cl_mem),(void *)&zoobj);

	global_size_item[0]=inputtriangles;

	ret=clEnqueueNDRangeKernel(cmdq,kernel,1,NULL,global_size_item,local_size_item,0,NULL,NULL); //CHECK 3RD ARGUEMENT PROPERLY! 1 if Not passing matrix
	ret=clFinish(cmdq);

	ret=clEnqueueReadBuffer(cmdq,xoobj,CL_TRUE,0,sizeof(float)*outputvertices,xo,0,NULL,NULL);
	ret=clEnqueueReadBuffer(cmdq,yoobj,CL_TRUE,0,sizeof(float)*outputvertices,yo,0,NULL,NULL);
	ret=clEnqueueReadBuffer(cmdq,zoobj,CL_TRUE,0,sizeof(float)*outputvertices,zo,0,NULL,NULL);
}

int main(int argc, char* argv[])
{
	//OpenGL Initialisation
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Curved Tetrahedron");

    InitializeGlutCallbacks();

    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }
    
    printf("GL version: %s\n", glGetString(GL_VERSION));
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//Depth Buffer
	
	glEnable(GL_DEPTH_TEST);

    CompileShaders();
	
	//OpenCL Startsd

	int iterations;

	printf("Number of Times:");
	scanf("%d",&iterations);
	
	xi = (float *)malloc(sizeof(float)*inputvertices);
	yi = (float *)malloc(sizeof(float)*inputvertices);
	zi = (float *)malloc(sizeof(float)*inputvertices);

	xi[0]=v0.x;		yi[0]=v0.y;		zi[0]=v0.z;
	xi[1]=v3.x;		yi[1]=v3.y;		zi[1]=v3.z;
	xi[2]=v1.x;		yi[2]=v1.y;		zi[2]=v1.z;

	xi[3]=v1.x;		yi[3]=v1.y;		zi[3]=v1.z;
	xi[4]=v3.x;		yi[4]=v3.y;		zi[4]=v3.z;
	xi[5]=v2.x;		yi[5]=v2.y;		zi[5]=v2.z;

	xi[6]=v2.x;		yi[6]=v2.y;		zi[6]=v2.z;
	xi[7]=v3.x;		yi[7]=v3.y;		zi[7]=v3.z;
	xi[8]=v0.x;		yi[8]=v0.y;		zi[8]=v0.z;

	xi[9]=v0.x;			yi[9]=v0.y;			zi[9]=v0.z;
	xi[10]=v1.x;		yi[10]=v1.y;		zi[10]=v1.z;
	xi[11]=v2.x;		yi[11]=v2.y;		zi[11]=v2.z;
	

	xo = (float *)malloc(sizeof(float)*outputvertices);
	yo = (float *)malloc(sizeof(float)*outputvertices);
	zo = (float *)malloc(sizeof(float)*outputvertices);
	

	//Kernel Loading
	FILE *fp;
	char *sourcestr;
	size_t sourcesize;
	fp=fopen("print.cl","r");
	if(!fp)
	{
		printf("Failed to load kernel");
		getchar();
		exit(1);
	}
	sourcestr=(char*)malloc(MAX_SOURCE);
	sourcesize=fread(sourcestr,1,MAX_SOURCE,fp);
	fclose(fp);
	//Kernel Loaded

	//OpenCL Init

	ret=clGetPlatformIDs(1,&platformid,&ret_numplatforms);

	ret=clGetDeviceIDs(platformid,CL_DEVICE_TYPE_GPU,1,&deviceid,&ret_numdevices);

	context=clCreateContext(NULL,1,&deviceid,NULL,NULL,&ret);

	cmdq=clCreateCommandQueue(context,deviceid,0,&ret);

	//OpenCL Init Done

	//Creating and Building Program. Creating Kernel
	program = clCreateProgramWithSource(context,1,(const char**)&sourcestr,(const size_t*)&sourcesize,&ret);
	ret=clBuildProgram(program,1,&deviceid,NULL,NULL,NULL);
	kernel=clCreateKernel(program,"hello_print",&ret);

	//Create Memory Buffers

	KernelExecution();		//Executes First Time
		
	//1st KERNEL EXECUTION FINISHED
		
	for(int i=1;i<iterations;i++)
	{
		inputtriangles=outputtriangles;
		inputvertices=outputvertices;
		outputtriangles=4*inputtriangles;
		outputvertices=3*outputtriangles;
		
		free(xi);	free(yi);	free(zi);

		printf("\nInput: %d, %d\nOutput: %d, %d\n\n",inputtriangles,inputvertices,outputtriangles,outputvertices);

		xi = (float *)malloc(sizeof(float)*inputvertices);
		yi = (float *)malloc(sizeof(float)*inputvertices);
		zi = (float *)malloc(sizeof(float)*inputvertices);

		for(int i=0;i<inputvertices;i++)
		{
			xi[i]=xo[i];
			yi[i]=yo[i];
			zi[i]=zo[i];
		}
		
		free(xo);	free(yo);	free(zo);

		xo = (float *)malloc(sizeof(float)*outputvertices);
		yo = (float *)malloc(sizeof(float)*outputvertices);
		zo = (float *)malloc(sizeof(float)*outputvertices);

		KernelExecution();
	}

	//for(int i=0;i<outputvertices;i++)
	//{
	//	printf("\nTriangle %d: %f, %f, %f",i/3,xo[i],yo[i],zo[i]);
	//}

	CreateVertexBuffer();
	
	glutMainLoop();
	
	ret=clFlush(cmdq);
	ret=clReleaseKernel(kernel);
	ret=clReleaseProgram(program);

	ret=clReleaseMemObject(xiobj);
	ret=clReleaseMemObject(yiobj);
	ret=clReleaseMemObject(ziobj);
	ret=clReleaseMemObject(xoobj);
	ret=clReleaseMemObject(yoobj);
	ret=clReleaseMemObject(zoobj);

	ret=clReleaseCommandQueue(cmdq);
	ret=clReleaseContext(context);

	return 0;
}
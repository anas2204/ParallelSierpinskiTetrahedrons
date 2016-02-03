__kernel void hello_print(__global float* xi, __global float* yi, __global float* zi,
						  __global float* xo, __global float* yo, __global float* zo)
{
	int index=get_global_id(0);
	int i=index*4;
	int j=index*16;

	xo[j+0]=xi[i+0];				yo[j+0]=yi[i+0];				zo[j+0]=zi[i+0];
	xo[j+1]=(xi[i+0]+xi[i+1])/2;	yo[j+1]=(yi[i+0]+yi[i+1])/2;	zo[j+1]=(zi[i+0]+zi[i+1])/2;
	xo[j+2]=(xi[i+0]+xi[i+2])/2;	yo[j+2]=(yi[i+0]+yi[i+2])/2;	zo[j+2]=(zi[i+0]+zi[i+2])/2;
	xo[j+3]=(xi[i+0]+xi[i+3])/2;	yo[j+3]=(yi[i+0]+yi[i+3])/2;	zo[j+3]=(zi[i+0]+zi[i+3])/2;
	
	xo[j+4]=xo[j+1];				yo[j+4]=yo[j+1];				zo[j+4]=zo[j+1];
	xo[j+5]=xi[i+1];				yo[j+5]=yi[i+1];				zo[j+5]=zi[i+1];
	xo[j+6]=(xi[i+1]+xi[i+2])/2;	yo[j+6]=(yi[i+1]+yi[i+2])/2;	zo[j+6]=(zi[i+1]+zi[i+2])/2;	
	xo[j+7]=(xi[i+1]+xi[i+3])/2;	yo[j+7]=(yi[i+1]+yi[i+3])/2;	zo[j+7]=(zi[i+1]+zi[i+3])/2;
	
	xo[j+8]=xo[j+2];				yo[j+8]=yo[j+2];				zo[j+8]=zo[j+2];
	xo[j+9]=xo[j+6];				yo[j+9]=yo[j+6];				zo[j+9]=zo[j+6];
	xo[j+10]=xi[i+2];				yo[j+10]=yi[i+2];				zo[j+10]=zi[i+2];
	xo[j+11]=(xi[i+2]+xi[i+3])/2;	yo[j+11]=(yi[i+2]+yi[i+3])/2;	zo[j+11]=(zi[i+2]+zi[i+3])/2;

	xo[j+12]=xo[j+3];				yo[j+12]=yo[j+3];				zo[j+12]=zo[j+3];
	xo[j+13]=xo[j+7];				yo[j+13]=yo[j+7];				zo[j+13]=zo[j+7];
	xo[j+14]=xo[j+11];				yo[j+14]=yo[j+11];				zo[j+14]=zo[j+11];	
	xo[j+15]=xi[i+3];				yo[j+15]=yi[i+3];				zo[j+15]=zi[i+3];
}
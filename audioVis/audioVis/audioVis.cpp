#include "stdafx.h"

#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>
#include <windows.h>
#include <WinUser.h>
#include <cstdlib>
#include <math.h>
#include <cmath>
#include <time.h>
#include <fstream>
#include <sndfile.h>
#include <sndfile.hh>
#include <vector>
#include <minmax.h>
#include "bass.h"
#include <thread>
#include <chrono>
#include <sstream>
using namespace std;
float randFloat();
float window_w = 1280;
float window_h = 720;
inline void drawScreen();
void changeSize(int w, int h)
{
	glClearAccum(0.0, 0.0, 0.0, 1.0);
	glClear(GL_ACCUM_BUFFER_BIT);
	window_w = w;
	window_h = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, h, 0);
	glEnable(GL_BLEND);
}
/*
SNDFILE *a;
SF_INFO ai;
float *data;

int datalen;

float newStart = 0;
int coolDown = 0;
//1/(dist / samplerate) * 60
float bestGuessBPM = 12;
float sd;
float osd;
vector<float> BPMS;
*/
/*
void renderScene()
{
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
/*glBegin(GL_LINE_LOOP);
for (int i = 0; i < datalen; i += 200)
{
glColor4f(1, 1, 1, 0.5);
glVertex2f(((float)i / (float)datalen)*1280.0f, 360.0f + data[(i * 2)] * 360.0f);
}
glVertex2f(1280, 720);
glVertex2f(0, 720);
glEnd();

glBegin(GL_LINES);
for (int i = 0; i < datalen; i += 1)
{
coolDown--;
if (data[i * 2] > 0.9 && coolDown <= 0)
{
coolDown = ai.samplerate / 8;
glColor4f(1, 0, 0, 1);
glVertex2f(((float)i / (float)datalen)*1280.0f, 0);
glVertex2f(((float)i / (float)datalen)*1280.0f, 16);
osd = sd;
sd = i;

bestGuessBPM = 1.0f / ((sd - osd) / (float)ai.samplerate) * 60.0f;
if (bestGuessBPM < 200);
BPMS.push_back(bestGuessBPM);
}
}
glEnd();

float average;
for (int i = 0; i < BPMS.size(); i++)
{
average += BPMS[i];
}
average /= BPMS.size();

if (average / 2 > 80 && average / 2 < 200)
average /= 2;

cout << "Average of all Points: " << average << endl;
float exclusiveAverage = 0;
int ec = 0;
for (int i = 0; i < BPMS.size(); i++)
{
if (abs(BPMS[i] - average) / BPMS[i] < 0.25)
{
ec++;
exclusiveAverage += BPMS[i];
}
}
exclusiveAverage /= ec;
cout << "exclusive Average of all Points: " << exclusiveAverage << endl;
bestGuessBPM = exclusiveAverage;



Sleep(1000);

glutSwapBuffers();
glutPostRedisplay();
}
*/

//in main
/*a = sf_open("a.wav", SFM_READ, &ai);
datalen = sf_seek(a, 0, SEEK_END) - sf_seek(a, 0, SEEK_SET);
data = new float[datalen * 2];
sf_read_float(a, data, datalen * 2);
cout << datalen << endl;
cout << "samplerate: " << ai.samplerate << endl;
cout << "channels:   " << ai.channels << endl;
cout << datalen / ai.samplerate << endl;
*/

//kick.loadSound("sounds\\kick.wav");

struct soundSample
{
	SNDFILE *openSound;
	SF_INFO soundInfo;
	float *data;
	int index;
	int playing;
	void loadSound(char *sound)
	{
		playing = 0;
		index = 0;
		openSound = sf_open(sound, SFM_READ, &soundInfo);
		cout << "Loading Sound: " << sound << "\n" << (unsigned char)195 << "Samples: " << soundInfo.frames << "\n" << (unsigned char)192 << "Channels: " << soundInfo.channels << "\n" << endl;
		data = new float[soundInfo.frames * soundInfo.channels];
		sf_readf_float(openSound, data, soundInfo.frames);
	}
};

soundSample kick;
soundSample rkick;
soundSample rclap;
HSTREAM streamHandle;

const int bufferLen = 512;
float globalBuffer[bufferLen * 2];
float ncglobalBuffer[bufferLen * 2];

std::vector<soundSample> queue;


struct CKeyboardController
{
	char oldKeys[256];
	char newKeys[256];

	CKeyboardController()
	{
		for (int i = 0; i < 256; i++)
		{
			oldKeys[i] = 0;
			newKeys[i] = 0;
		}
	}
	void run()
	{
		for (int i = 0; i < 256; i++)
		{
			oldKeys[i] = newKeys[i];
			newKeys[i] = !!GetAsyncKeyState(i);
		}
	}
	bool keyDown(int key)
	{
		return newKeys[key];
	}
	bool keyDownEvent(int key)
	{
		return newKeys[key] && !oldKeys[key];
	}
};
CKeyboardController *keyboard = new CKeyboardController();
int commitedSamples = 0;
void playBuffer(int vari)
{
	int qq = 0;
	for (int a = 0; a < queue.size(); a++)
	{
		if (queue[a].playing)
		{
			if (queue[a].index < queue[a].soundInfo.frames * 2)
			{
				for (int i = 0; i < (bufferLen * 2) - vari; i++)
				{
					if (queue[a].index > queue[a].soundInfo.frames * 2)
						break;
					globalBuffer[i] += queue[a].data[queue[a].index];
					queue[a].index++;
					commitedSamples++;
					qq = 1;
				}
			}
			else
			{
				queue[a].playing = 0;
				//cout << "Done...\n";
			}
		}
	}
	if (qq)
	{
		BASS_StreamPutData(streamHandle, globalBuffer, bufferLen * sizeof(float) * 2);
		BASS_ChannelPlay(streamHandle, FALSE);
	}
	for (int i = 0; i < bufferLen * 2; i++)
	{
		ncglobalBuffer[i] = globalBuffer[i];
		globalBuffer[i] = 0;
	}
}
void playSound(soundSample *a)
{
	a->playing = 1;
	queue.push_back(*a);
}

int q = 0;

int jitter = 0;
void soundThread()
{
	while (1)
	{
		this_thread::sleep_for(chrono::milliseconds((int)(((bufferLen) / 44100.0f) * 1000) - 2));
		playBuffer(0);
	}
}

float maxf[4096];
float gfft[4096];

float energy = 0;

struct part
{
	part(float nx, float ny, float nxv, float nyv, int ressonant)
	{
		x = nx;
		y = ny;
		xv = nxv;
		yv = nyv;
		freq = ressonant;
	}

	void draw()
	{
		x += xv * energy;
		y += yv * energy;

		if (x > 1280)
			x = 0;
		if (x < 0)
			x = 1280;

		if (y > 720)
			y = 0;
		if (y < 0)
			y = 720;


		glColor4f(1, 1, 1, 0.01 + (gfft[freq] * 10) + (energy / 20.0f));
		glVertex2f(x, y);
	}

	float x;
	float y;
	float xv;
	float yv;
	int freq;
};
vector<part> parts;
void drawFFT(float alpha)
{
	const int buf = 4096;
	float fft[4096];
	BASS_ChannelGetData(streamHandle, &fft, BASS_DATA_FFT4096);
	float fract = 1280 / 2;
	float sp = fract / 512;
	float max = buf / 2;
	energy = 0;
	for (int i = 0; i < buf / 2; i++)
	{
		gfft[i] = fft[i];
		maxf[i] = max(maxf[i], (fft[int(i)] * (0.06f + (i * 4 / max))  * 32.0f) * 300.0f);
		maxf[i] /= 1.02;
		energy += fft[i];
	}
	energy /= (buf / 2);
	energy *= 400;
	energy = min(4.0f, energy);
	//cout << energy << endl;

	glLineWidth(8);
	glBegin(GL_LINE_LOOP);
	glColor4f(0, 0, 0, 0.1);
	glVertex2f(0, 7200);
	for (int i = 0; i < buf / 2; i++)
	{
		float a = min(maxf[i] * 0.025, 1);

		glColor4f(a, a, a, alpha / 10.0f);
		glVertex2f(log(1 + (i / 10.0f)) * 245.0f, -maxf[i]);
	}
	glColor3f(0, 0, 0);
	glVertex2f(12800, 7200);
	glEnd();

	glLineWidth(4);
	glBegin(GL_LINE_LOOP);
	glColor4f(0, 0, 0, 0.1);
	glVertex2f(-1000, 7200);
	for (int i = 0; i < buf / 2; i++)
	{
		float a = min(maxf[i] * 0.025, 1);

		glColor4f(a, a, a, alpha / 2.5);
		glVertex2f(log(1 + (i / 10.0f)) * 245.0f, -maxf[i]);
	}
	glColor3f(0, 0, 0);
	glVertex2f(12800, 720);
	glEnd();

	glLineWidth(1);
	glBegin(GL_LINE_LOOP);
	glColor3f(0, 0, 0);
	glVertex2f(-1000, 7200);
	for (int i = 0; i < buf / 2; i++)
	{
		float a = min(maxf[i] * 0.025, 1);

		glColor4f(a, a, a, alpha);
		glVertex2f(log(1 + (i / 10.0f)) * 245.0f, -maxf[i]);
	}
	glColor3f(0, 0, 0);
	glVertex2f(12800, 7200);
	glEnd();
}

inline void fillCircle(int iter)
{
	float ang = 2.0f * 3.141 / iter;
	glBegin(GL_POLYGON);
	for (int i = 0; i < iter; ++i)
	{
		glVertex2f(cos(i*ang), sin(i*ang));
	}
	glEnd();

}
inline void lineCircle(int iter)
{
	float ang = 2.0f * 3.141 / iter;
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < iter; ++i)
	{
		glVertex2f(cos(i*ang), sin(i*ang));
	}
	glVertex2f(cos(0*ang), sin(0*ang));
	glEnd();

}
inline void fillRect()
{
	glBegin(GL_POLYGON);
	glVertex2f(0, 0);
	glVertex2f(1, 0);
	glVertex2f(1, 1);
	glVertex2f(0, 1);
	glEnd();
}

struct spinc
{
	float rot;
	float rots;
	float radius;
	float dir;
	spinc()
	{
		rot = 0;
		radius = rand() % 1000;
		rots = randFloat() * 2;		
		dir = rand() % 1 ? 1 : -1;
	}
	spinc(float _rot, float _radius)
	{
		rot = _rot;
		radius = _radius;
		rots = rand() % 1000 / 1000;
		dir = rand() % 1 ? 1 : -1;
	}
	void changeDir()
	{
		dir *= -1;
	}
	void update()
	{
		rot += rots * dir;
	}
};



float rot = -3.141/2;
float rots = 0;
float randColour[3] = { 0.0f };
float beats = 0;
bool beaton = true;
bool oldbeat = false;
int dir = 1;
spinc * spins = new spinc[10];
bool drawSpins = false;
int timeout = 0;
float fft[4096];
float tfft[4096];
float bassEnergy = 1;
float totalEnergy = 1;
bool highEnergy = true;
vector<spinc*> spin;
int state = 0;
bool bubbles = 1;
bool dark = 1;
bool linear = 1;
bool circle = 1;
bool circles = 1;
bool circlesr = 1;
bool hexs = 1;
bool spinning = 1;
bool glow = 1;
bool blank = 0;
int spread = 1;
bool motionBlur = 1;
float motionblur = 0.85f;

float pos[4096];
float osc = 0;
spinc spinp[2048];
float osc2 = 0;

int brightness = 20;
float sharpness = 5;
const int buf = 4096;

bool waitFlag = 0;

void printOut()
{
	printf("bubbles %i dark %i linear %i circle %i circles %i circlesr %i hexs %i spinning %i glow %i\n",
		bubbles, dark, linear, circle, circles, circlesr, hexs, spinning, glow);
}
void printFFT()
{
	for (int i = 0; i < buf / 2; ++i)
		cout << gfft[i] << " : " << (!(i % 3) ? "\n" : "");
}
void processThread()
{
	//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	while (1)
	{		
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
			keyboard->run();
			if (keyboard->keyDownEvent(0x38))
				bubbles = !bubbles;
			if (keyboard->keyDownEvent(0x31))
				dark = !dark;
			if (keyboard->keyDownEvent(0x32))
				linear = !linear;
			if (keyboard->keyDownEvent(0x33))
				circle = !circle;
			if (keyboard->keyDownEvent(0x34))
				circles = !circles;
			if (keyboard->keyDownEvent(0x35))
				circlesr = !circlesr;
			if (keyboard->keyDownEvent(0x36))
				hexs = !hexs;
			if (keyboard->keyDownEvent(0x37))
				spinning = !spinning;
			if (keyboard->keyDownEvent(0x39))
				glow = !glow;
			if (keyboard->keyDownEvent(0x30))
				motionBlur = !motionBlur;
			if (keyboard->keyDownEvent(VK_RETURN))
			{
				bubbles = 0;
				dark = 0;
				linear = 0;
				circle = 0;
				circles = 0;
				circlesr = 0;
				hexs = 0;
				spinning = 0;
				glow = 0;
				motionBlur = 0;
			}
			if (keyboard->keyDown(0x5A))
				brightness += 1;
			if (keyboard->keyDown(0x58))
				brightness -= 1;
			if (keyboard->keyDown(0x43))
				sharpness += 0.0001f;
			if (keyboard->keyDown(0x56))
				sharpness -= 0.0001f;
			//if (keyboard->keyDown(VK_SPACE))
			//	printFFT();



			float fract = 1280 / 2;
			float max = buf / 2;
			float sp = fract / max;
			BASS_ChannelGetData(streamHandle, &fft, BASS_DATA_FFT4096);

			

			int cuttoff = 30;
			bassEnergy = 0;
			for (int i = 0; i < cuttoff; i++)
				bassEnergy += fft[i];
			bassEnergy /= cuttoff;
			for (int i = 0; i < max; i++)
				totalEnergy += fft[i];
			totalEnergy /= max;
			highEnergy = totalEnergy > 0.0005;

			timeout--;
			if (timeout < 0) beats = 0;
			if (bassEnergy < 0.05)
			{
				oldbeat = beaton;
				beaton = false;
			}
			else
			{
				oldbeat = beaton;
				beaton = true;
			}

			if (beaton && !oldbeat)
			{
				cout << beats << timeout << endl;
				timeout = 100;
				highEnergy = totalEnergy > 0.002;
				if (highEnergy)
				{
					randColour[0] = { randFloat() + 0.6f };
					randColour[1] = { randFloat() + 0.6f };
					randColour[2] = { randFloat() + 0.6f };
				}
				//if (false)
					if (beats == 0)
					{
					//dir *= -1;
					bubbles = rand() % 2;
					dark = rand() % 2;
					linear = rand() % 2;
					circle = rand() % 2;
					circles = rand() % 2;
					circlesr = rand() % 2;
					hexs = rand() % 2;
					spinning = rand() % 2;
					glow = rand() % 2;
					blank = rand() % 2;
					spread = rand() % 5;
					motionBlur = rand() % 2;

					if (bubbles + dark + linear + circle + circles + hexs + spinning == 0)
						spinning = 1;
					spin.clear();
					printOut();
					}
				beats = beats == 15 ? beats = 0 : beats + 1;
				if ((int(beats) % 4) == 0)
					for (int i = 0; i < 10; ++i)
						spins[i].dir *= -1;

				spin.push_back(new spinc((rand() % 1000) / 1000 * 3.141, dark ? window_w : 1));

			}

			bassEnergy /= 2;
			for (int i = 0; i < max; i++)
			{
				tfft[i] = 0;
				int space = i + 1;
				for (int j = 0; i + j < max && j < space; j++)
					tfft[i] += fft[i + j];
				tfft[i] /= space;
				//tfft[i] = fft[i];
			}

			if (!waitFlag)
				for (int i = 0; i < max; i++)
					gfft[i] = tfft[i];

			for (int i = max - 1; i >= 0; i--)
			{
				spinp[i].radius += spinp[i].radius * bassEnergy * 2;
				spinp[i].rot += bassEnergy * (circlesr ? 1 : -1) / 2;
				if (spinp[i].radius > window_w)
				{
					spinp[i].radius = rand() % int(window_w);
					spinp[i].rot = randFloat()*3.141f;
				}
			}
		
			osc += 0.02f * (circlesr ? -1 : 1);
	}
}

inline void drawSpinningParts()
{
	float max = buf / 2 / 2;
//#pragma omp parallel for
	for (int i = max - 1; i >= 0; i--)
	{
		//float colour = gfft[i] * 500;
		//glColor4f(colour * randColour[0], colour *randColour[1], colour * randColour[2], 1.0f);
		//glColor3f(colour * randColour[0], colour *randColour[1], colour * randColour[2]);
		glColor3f( randColour[0],randColour[1], randColour[2]);
		glPushMatrix();
		glTranslatef(spinp[i].radius*cos(spinp[i].rot), spinp[i].radius*sin(spinp[i].rot), 0);
		//glScalef(spinp[i].radius / 100 + 1, spinp[i].radius / 100 + 1, 0);
		fillRect();
		glPopMatrix();
	}
}
inline void drawBubbles()
{
	float max = buf / 2 / 2;
	int k = 0;
	float size = 400 * bassEnergy;
	float size2 = size / 2;
//#pragma omp parallel for
	for (int i = max - 1; i >= 0; i -= 2)
	{		
		//float colour = gfft[i / 2] * 500;
		//glColor4f(colour * randColour[0], colour *randColour[1], colour * randColour[2], 1.0f);
		//glColor3f(colour * randColour[0], colour *randColour[1], colour * randColour[2]);
		glColor3f(randColour[0], randColour[1], randColour[2]);
		glPushMatrix();
		glTranslatef((pos[i]) * window_w - size2 + cos(osc)*size*pos[i] * 10, (pos[i + 1]) * window_h - size2 + sin(osc)*size*pos[i] * 10, 0);
		glScalef(size, size, 0);
		fillCircle(7);
		glPopMatrix();
	}
}
inline void drawDark()
{
	float max = buf / 2;
	float radius = bassEnergy * 500 + 100;

	radius *= 0.9f;
	float newb = bassEnergy * 5;
	//glColor3f(energy*randColour[0], energy*randColour[1], energy*randColour[2]);
	//glColor4f(0, 0, 0, 1.0f);
	glColor3f(0, 0, 0);
	glPushMatrix();
	glScalef(radius, radius, 0);
	fillCircle(50);
	glPopMatrix();

	//glColor4f(1, 1, 1, 1.0f);
	glColor3f(1, 1, 1);
	glBegin(GL_LINES);
	for (int i = 0; i < max; i++)
	{
		float theta = 0.6f*(log(1 + (i / 10.0f))) + rot;

		float x1 = cos(theta) * radius;
		float y1 = sin(theta) * radius;
		glVertex2f(x1, y1);

		float x2 = cos(theta) * (radius + -(gfft[int(i)] * (0.06f + (i * 4 / max))  * 32.0f) * 150.0f);
		float y2 = sin(theta) * (radius + -(gfft[int(i)] * (0.06f + (i * 4 / max))  * 32.0f) * 150.0f);
		glVertex2f(x2, y2);
	}
	//if (true)
	for (int i = 0; i < max; i++)
	{
		float theta = 0.6f * -(log(1 + (i / 10.0f))) + rot;

		float x1 = cos(theta) * radius;
		float y1 = sin(theta) * radius;
		glVertex2f(x1, y1);

		float x2 = cos(theta) * (radius + -(gfft[int(i)] * (0.06f + (i * 4 / max))  * 32.0f) * 150.0f);
		float y2 = sin(theta) * (radius + -(gfft[int(i)] * (0.06f + (i * 4 / max))  * 32.0f) * 150.0f);
		glVertex2f(x2, y2);
	}
	glEnd();
}
inline void drawHexs()
{
	if (hexs)
		for (int i = 0; i < 10; ++i)
		{
		spins[i].update();
		//glColor4f(bassEnergy * 20, bassEnergy * 20, bassEnergy * 20, 1.0f);
		glColor3f(bassEnergy * 20, bassEnergy * 20, bassEnergy * 20);
		glPushMatrix();
		glRotatef(spins[i].rot, 0, 0, 1);
		glScalef(spins[i].radius, spins[i].radius, 0);
		lineCircle(int(bassEnergy * 100) + 3);
		glPopMatrix();
		}
}
inline void drawCircle()
{
	float max = buf / 2;
	float radius = bassEnergy * 1000 + 100;

	//glColor4f(randColour[0], randColour[1], randColour[2], 1.0f);
	glColor3f(randColour[0], randColour[1], randColour[2]);
	glBegin(GL_LINES);
	//glBegin(GL_LINE_STRIP);
	//glBegin(GL_POLYGON);
	for (int i = 0; i < max; i++)
	{
		float theta = 0.6f*(log(1 + (i / 10.0f))) + rot;
		float fftt = gfft[int(i)] * (0.06f + (i * 4 / max))  * 32.0f * 300.0f;

		float x1 = cos(theta) * radius;
		float y1 = sin(theta) * radius;
		glVertex2f(x1, y1);

		float x2 = cos(theta) * (radius + fftt);
		float y2 = sin(theta) * (radius + fftt);
		glVertex2f(x2, y2);

		/*float x3 = cos(theta) * (radius + fftt*0.1f);
		float y3 = sin(theta) * (radius + fftt*0.1f);
		glVertex2f(x3, y3);*/
	}
	for (int i = max - 1; i >= 0; i--)
	{
		float theta = 0.6f * -(log(1 + (i / 10.0f))) + rot;
		float fftt = gfft[int(i)] * (0.06f + (i * 4 / max))  * 32.0f * 300.0f;

		float x1 = cos(theta) * radius;
		float y1 = sin(theta) * radius;
		glVertex2f(x1, y1);

		float x2 = cos(theta) * (radius + fftt);
		float y2 = sin(theta) * (radius + fftt);
		glVertex2f(x2, y2);

		/*float x3 = cos(theta) * (radius + fftt*0.1f);
		float y3 = sin(theta) * (radius + fftt*0.1f);
		glVertex2f(x3, y3);*/
	}
	glEnd();
}
inline void drawCircles()
{
	//glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xAAAA);
	glLineWidth(8);
	for (int i = 0; i < spin.size(); ++i)
	{
		if (spin[i] != nullptr)
		{
			spin[i]->radius += spin[i]->radius * bassEnergy * 2 * (circlesr ? -0.5 : 1);
			float e = bassEnergy * spin[i]->radius / 2;
			//glColor4f(e, e, e, 1.0f);
			glColor3f(e, e, e);
			glPushMatrix();
			glScalef(spin[i]->radius, spin[i]->radius, 0);
			lineCircle(50);
			glPopMatrix();
		}
	}
	if (spin.size() > 0 && (spin[spin.size() - 1]->radius > 4096 || spin[0]->radius < 1))
		spin.pop_back();
	glLineWidth(2);
}
inline void drawLinear()
{
	float max = buf / 2;
	//glColor4f(randColour[0], randColour[1], randColour[2], 1.0f);
	glColor3f(randColour[0], randColour[1], randColour[2]);
	glBegin(GL_LINES);
	//glBegin(GL_LINE_STRIP);
	//glBegin(GL_POLYGON);
	float carry = 0;
	for (int i = 0; i < max; i++)
	{
		float theta = 0.335f*(log(1 + (i / 10.0f)));
		float fftt = gfft[i] * sqrt(i);

		float x1 = theta;
		float y1 = fftt*0.8f;
		glVertex2f(x1, y1);

		float x2 = theta;
		float y2 = fftt;
		glVertex2f(x2, y2);
	}
	glEnd();
}

void drawText(float x, float y, string text)
{
	glRasterPos2f(x, y);
	for (int i = 0; i < text.length(); i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, text[i]);
	}
}

int textureSize = 720;
GLuint BlurTexture;
GLuint GlowTexture;
int textures = 10;
GLuint text[10];
int textID = 0;
GLuint EmptyTexture()                           // Create An Empty Texture
{
	GLuint txtnumber;                       // Texture ID
	unsigned int* data;                     // Stored Data

	// Create Storage Space For Texture Data (128x128x4)
	data = (unsigned int*)new GLuint[((textureSize * textureSize) * 4 * sizeof(unsigned int))];
	ZeroMemory(data, ((textureSize * textureSize) * 4 * sizeof(unsigned int)));   // Clear Storage Memory

	glGenTextures(1, &txtnumber);                   // Create 1 Texture
	glBindTexture(GL_TEXTURE_2D, txtnumber);            // Bind The Texture
	glTexImage2D(GL_TEXTURE_2D, 0, 4, textureSize, textureSize, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);           // Build Texture Using Information In data
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete[] data;                         // Release data

	return txtnumber;                       // Return The Texture ID
}
GLuint EmptyTexture2()                           // Create An Empty Texture
{
	GLuint txtnumber;                       // Texture ID
	unsigned int* data;                     // Stored Data

	// Create Storage Space For Texture Data (128x128x4)
	data = (unsigned int*)new GLuint[((window_w * window_h) * 4 * sizeof(unsigned int))];
	ZeroMemory(data, ((window_w * window_h) * 4 * sizeof(unsigned int)));   // Clear Storage Memory

	glGenTextures(1, &txtnumber);                   // Create 1 Texture
	glBindTexture(GL_TEXTURE_2D, txtnumber);            // Bind The Texture
	glTexImage2D(GL_TEXTURE_2D, 0, 4, window_w, window_h, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);           // Build Texture Using Information In data
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete[] data;                         // Release data

	return txtnumber;                       // Return The Texture ID
}
inline void ViewOrtho()                            
{
	glMatrixMode(GL_PROJECTION);                  
	glPushMatrix();                         
	glLoadIdentity();                       
	glOrtho(0, window_w, window_h, 0, -1, 1);             
	glMatrixMode(GL_MODELVIEW);                 
	glPushMatrix();                        
	glLoadIdentity();                       
}
inline void ViewPerspective()                         
{
	glMatrixMode(GL_PROJECTION);                
	glPopMatrix();                         
	glMatrixMode(GL_MODELVIEW);                  
	glPopMatrix();                        
}
inline void RenderToTexture()                         
{
	glViewport(0, 0, textureSize, textureSize);
	drawScreen();
	glBindTexture(GL_TEXTURE_2D, BlurTexture);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, textureSize, textureSize, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glViewport(0, 0, window_w, window_h);
}
inline void drawGlow(float dist, float levels)
{
	int x = 0;
	int  y = 0;
	int w = window_w;
	int h = window_h;

	// Disable AutoTexture Coordinates
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glEnable(GL_TEXTURE_2D);                    // Enable 2D Texture Mapping
	glDisable(GL_DEPTH_TEST);                   // Disable Depth Testing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);               // Set Blending Mode
	glEnable(GL_BLEND);                     // Enable Blending
	glBindTexture(GL_TEXTURE_2D, GlowTexture);
	ViewOrtho();

		for (int i = 0; i < levels; i++)
		{
			
			float alpha = (1.0f - i / (levels) )* (totalEnergy)* 50;
			glPushMatrix();
			glTranslatef(0, h + i * 2 * dist, 0);
			glTranslatef(x - i * dist, y - i * dist, 0);
			glScalef(w + i * 2 * dist, h + i * 2 * dist, 0);
			glScalef(1, -1, 0);
				glBegin(GL_QUADS);
				glColor4f(1, 1, 1, alpha);
				glTexCoord2f(0, 0);
				glVertex2f(0, 0);
				glTexCoord2f(1, 0);
				glVertex2f(1, 0);
				glTexCoord2f(1, 1);
				glVertex2f(1, 1);
				glTexCoord2f(0, 1);
				glVertex2f(0, 1);
			glEnd();
			glPopMatrix();
		}	

	ViewPerspective();
	glEnable(GL_DEPTH_TEST);                    // Enable Depth Testing
	glDisable(GL_TEXTURE_2D);                   // Disable 2D Texture Mapping
	glDisable(GL_BLEND);                        // Disable Blending
	glBindTexture(GL_TEXTURE_2D, 0);

}
inline void drawBlur(float dist, float levels)
{
	int x = 0;
	int  y = 0;
	int w = window_w;
	int h = window_h;

	// Disable AutoTexture Coordinates
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glEnable(GL_TEXTURE_2D);                    // Enable 2D Texture Mapping
	glDisable(GL_DEPTH_TEST);                   // Disable Depth Testing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);               // Set Blending Mode
	glEnable(GL_BLEND);                     // Enable Blending
	glBindTexture(GL_TEXTURE_2D, GlowTexture);
	ViewOrtho();

	for (int i = 0; i < levels; i++)
	{;

		float alpha = (1.0f - i / (levels));
		glPushMatrix();
		glTranslatef(0, h, 0);
		glTranslatef(x + dist * randFloat(), y + dist * randFloat(), 0);
		glScalef(w, h, 0);
		glScalef(1, -1, 0);
		glBegin(GL_QUADS);
		glColor4f(1, 1, 1, 0.5f);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glTexCoord2f(1, 0);
		glVertex2f(1, 0);
		glTexCoord2f(1, 1);
		glVertex2f(1, 1);
		glTexCoord2f(0, 1);
		glVertex2f(0, 1);
		glEnd();
		glPopMatrix();
	}

	ViewPerspective();
	glEnable(GL_DEPTH_TEST);                    // Enable Depth Testing
	glDisable(GL_TEXTURE_2D);                   // Disable 2D Texture Mapping
	glDisable(GL_BLEND);                        // Disable Blending
	glBindTexture(GL_TEXTURE_2D, 0);

}
inline void drawMotionBlur(float mb)
{
	int x = 0;
	int  y = 0;
	int w = window_w;
	int h = window_h;

	// Disable AutoTexture Coordinates
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glEnable(GL_TEXTURE_2D);                    // Enable 2D Texture Mapping
	glDisable(GL_DEPTH_TEST);                   // Disable Depth Testing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);               // Set Blending Mode
	glEnable(GL_BLEND);                     // Enable Blending
	//glBindTexture(GL_TEXTURE_2D, GlowTexture);
	ViewOrtho();

	//for (int i = 0; i < textures; i++)
	{
		glBindTexture(GL_TEXTURE_2D, BlurTexture);
		glPushMatrix();
		glTranslatef(0, h, 0);
		glTranslatef(x , y , 0);
		glScalef(w, h , 0);
		glScalef(1, -1, 0);
		glBegin(GL_QUADS);
		glColor4f(1, 1, 1, mb);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glTexCoord2f(1, 0);
		glVertex2f(1, 0);
		glTexCoord2f(1, 1);
		glVertex2f(1, 1);
		glTexCoord2f(0, 1);
		glVertex2f(0, 1);
		glEnd();
		glPopMatrix();
	}

	ViewPerspective();
	glEnable(GL_DEPTH_TEST);                    // Enable Depth Testing
	glDisable(GL_TEXTURE_2D);                   // Disable 2D Texture Mapping
	glDisable(GL_BLEND);                        // Disable Blending
	glBindTexture(GL_TEXTURE_2D, 0);
}

inline void renderSceneToGlowTex()
{
	drawScreen();	
	glBindTexture(GL_TEXTURE_2D, GlowTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_w, window_h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, window_w, window_h, 0);

	drawMotionBlur(motionblur);
	glBindTexture(GL_TEXTURE_2D, BlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_w, window_h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, window_w, window_h, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

inline void drawScreen()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glLineWidth(2);
	if (false)
		if (!(state == 1) && highEnergy)
		{
		glPushMatrix();
		glScalef(window_w, window_h, 0);
		glBegin(GL_POLYGON);
		glColor4f(1 - randColour[0], 1 - randColour[1], 1 - randColour[2], 1.0f);

		glVertex2f(0, 0);
		glVertex2f(1, 0);
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glVertex2f(1, 1);
		glVertex2f(0, 1);
		glEnd();
		glPopMatrix();
		}

	glPushMatrix();
	glTranslatef(window_w / 2, window_h / 2, 0);
	glScalef(1, 1, 0);
	if (highEnergy && spinning)
		drawSpinningParts();
	if (bubbles)
		drawBubbles();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(window_w / 2, window_h / 2, 0);
	if (hexs)
		drawHexs();
	if (highEnergy && (circles))
		drawCircles();	
	if (circle)
		drawCircle();
	if (dark)
		drawDark();
	glPopMatrix();

	if (linear)
	{
		glPushMatrix();
		glTranslatef(0, window_h, 0);
		glScalef(window_h / 2, -window_h, 0);
		drawLinear();
		glPopMatrix();
		glPushMatrix();
		glTranslatef(window_w, window_h, 0);
		glScalef(-window_h / 2, -window_h, 0);
		drawLinear();
		glPopMatrix();
	}
}

void renderScene()
{
	//processThread();
	
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

	waitFlag = 1;
	renderSceneToGlowTex();
	waitFlag = 0;

	if (blank || !glow)
	drawBlur(pow(2, spread), 15);
	if (glow)
	drawGlow(sharpness, brightness);
	if (motionBlur)
	drawMotionBlur(motionblur);

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();	
}

inline float randFloat()
{
	return ((rand() % 1000) - (rand() % 1000)) / 1000.0f;
}

void testInit()
{

	GlowTexture = EmptyTexture2();
	BlurTexture = EmptyTexture2();

	for (int i = 0; i < textures; ++i)
		text[i] = EmptyTexture2();

	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);								

	GLfloat global_ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };		
	GLfloat light0pos[4] = { 0.0f, 5.0f, 10.0f, 1.0f };		
	GLfloat light0ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };		
	GLfloat light0diffuse[4] = { 0.3f, 0.3f, 0.3f, 1.0f };		
	GLfloat light0specular[4] = { 0.8f, 0.8f, 0.8f, 1.0f };		

	GLfloat lmodel_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };			
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);		

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);		
	glLightfv(GL_LIGHT0, GL_POSITION, light0pos);				
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0ambient);			
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0diffuse);			
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0specular);			
	//glEnable(GL_LIGHTING);										
	//glEnable(GL_LIGHT0);										

	//glShadeModel(GL_SMOOTH);									

	//glMateriali(GL_FRONT, GL_SHININESS, textureSize);
	//glClearColor(0.0f, 0.0f, 0.0f, 0.5);
}

int main(int argc, char **argv)
{
	srand(time(0));
	rand();	rand();	rand();

	for (int i = 0; i < 4096; i += 2)
	{
		pos[i] =  randFloat();
		pos[i + 1] =  randFloat();
	}

	for (int i = 0; i < 2048; i++)
	{
		spinp[i] = spinc((rand() % 1000) /1000 *3.141*2, rand() % int(window_w));
	}

	BASS_Init(-1, 44100, 0, 0, NULL);
	cout << BASS_ErrorGetCode() << endl;


	if (argc > 1)
	{
		cout << argv[1] << endl;
		q = 1;
	}
	keyboard->run();
	if (!q)
	{
		std::thread sound(soundThread);
		sound.detach();
	}

	if (!q)
	{
		streamHandle = BASS_StreamCreateFile(0, "d.mp3", 0, 0, 0);
		BASS_ChannelPlay(streamHandle, 0);
	}
	else
	{
		streamHandle = BASS_StreamCreateFile(0, argv[1], 0, 0, 0);
		BASS_ChannelPlay(streamHandle, 0);
	}
	cout << BASS_ErrorGetCode() << endl;

	std::thread process(processThread);
	process.detach();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(window_w, window_h);
	glutCreateWindow("");
	glEnable(GL_SMOOTH);
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	//glutIdleFunc(processThread);

	testInit();

	//glLineWidth(2);
	//glEnable(GL_LINE_STIPPLE);
	//glLineStipple(2, 0xAAAA);

	glutMainLoop();
	BASS_Free();

	getc(stdin);
	return 0;
}
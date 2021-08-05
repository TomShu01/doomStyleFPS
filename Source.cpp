#include <iostream>
using namespace std;

#include <Windows.h> // to grab the console
#include <chrono>

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f; // angle the player is looking at

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;

int main() {
	// create screen buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight]; // screen is basically an array of pixels
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);// make our screen buffer as the buffer used by the console
	DWORD dwBytesWritten = 0;// just a useless variable from windows

	wstring map;

	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"######...#######";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	while (1) {
		// timing
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		// controls
		// handle CCW Rotation
		if (GetAsyncKeyState((unsigned short)'\x25') & 0x8000) { // left arrow
			fPlayerA -= (1.0f) * fElapsedTime;
		}

		// handle CW rotation
		if (GetAsyncKeyState((unsigned short)'\x27') & 0x8000) { // right arrow
			fPlayerA += (1.0f) * fElapsedTime;
		}

		// handle forward and backwards walking
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			float nextX = fPlayerX + sinf(fPlayerA) * 5.0f * fElapsedTime;
			float nextY = fPlayerY + cosf(fPlayerA) * 5.0f * fElapsedTime;
			if (map[(int)nextY * nMapWidth + (int)nextX] != '#') { // collision detection code
				fPlayerX = nextX; // i think the default angle is facing south
				fPlayerY = nextY;
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			float nextX = fPlayerX - sinf(fPlayerA) * 5.0f * fElapsedTime;
			float nextY = fPlayerY - cosf(fPlayerA) * 5.0f * fElapsedTime;
			if (map[(int)nextY * nMapWidth + (int)nextX] != '#') { // collision detection code
				fPlayerX = nextX; // i think the default angle is facing south
				fPlayerY = nextY;
			}
		}

		// handle strafing left and right
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			float left_angle = fPlayerA - 3.14159 / 2;
			float nextX = fPlayerX + sinf(left_angle) * 5.0f * fElapsedTime;
			float nextY = fPlayerY + cosf(left_angle) * 5.0f * fElapsedTime;
			if (map[(int)nextY * nMapWidth + (int)nextX] != '#') { // collision detection code
				fPlayerX = nextX; // i think the default angle is facing south
				fPlayerY = nextY;
			}
		}

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			float right_angle = fPlayerA + 3.14159 / 2;
			float nextX = fPlayerX + sinf(right_angle) * 5.0f * fElapsedTime;
			float nextY = fPlayerY + cosf(right_angle) * 5.0f * fElapsedTime;
			if (map[(int)nextY * nMapWidth + (int)nextX] != '#') { // collision detection code
				fPlayerX = nextX; // i think the default angle is facing south
				fPlayerY = nextY;
			}
		}

		// ray tracing to get our first person view
		for (int x = 0; x < nScreenWidth; x++) {
			// doom style graphics is really just a 2d problem, so we only have to worry about each column of the screen
			// for every pixel on our FOV, we will shoot out a ray, that increments until it hits a wall

			// for each column, calculate the project ray angle into world space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			float fDistanceToWall = 0;
			bool bHitWall = false;

			// unit vectors of a ray that's shot out from an angle
			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth) {// sometimes, we don't hit a wall, we only test until the depth has been reached
				// first, get distance to wall
				fDistanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);// cast to int so we can test it on our map
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
					bHitWall = true; //if going beyond bounds of map, we just say that you've hit a wall'
					fDistanceToWall = fDepth;
				}
				else {
					// if we reach a '#' then we've hit a wall
					if (map[nTestY * nMapWidth + nTestX] == '#') {
						bHitWall = true;
					}
				}

				// second, calculate half of height of wall in the distance, then use that to calculate height of ceiling and floor in the distance
				//float halfOfWall = ;
				int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
				int nFloor = nScreenHeight - nCeiling;

				short nShade = ' ';// shade the wall depending on how far it is

				// shades from very close to very far away
				if (fDistanceToWall <= fDepth / 4.0f) {
					nShade = 0x2588;
				}
				else if (fDistanceToWall < fDepth / 3.0f) {
					nShade = 0x2593;
				}
				else if (fDistanceToWall < fDepth / 2.0f) {
					nShade = 0x2592;
				}
				else if (fDistanceToWall < fDepth) {
					nShade = 0x2591;
				}
				else {
					nShade = ' ';
				}

				// lastly, update our screen with how our wall shall look
				for (int y = 0; y < nScreenHeight; y++) {
					if (y < nCeiling) {
						screen[y * nScreenWidth + x] = ' ';
					}
					else if (y >= nCeiling && y <= nFloor) {
						screen[y * nScreenWidth + x] = nShade;
					}
					else {
						// b is the ratio of distance from y to bottom of screen and half of screen height
						float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
						// the following code shades the floor
						if (b < 0.25) {// using 0.25 because we are using floats
							nShade = '#';
							//nShade = nShade;
						}
						else if (b < 0.5) {
							nShade = 'x';
							//nShade = nShade;
						}
						else if (b < 0.75) {
							nShade = '.';
							//nShade = nShade;
						}
						else if (b < 0.9) {
							nShade = '-';
							//nShade = nShade;
						}
						else {
							nShade = ' ';
							//nShade = nShade;
						}
						screen[y * nScreenWidth + x] = nShade;
					}
				}
			}
		}

		// HUD: heads-up display

		// Display stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime); //Elapsed time is T per frame, so FPS = F = 1/T

		// Display Map
		for (int nx = 0; nx < nMapWidth; nx++) {
			for (int ny = 0; ny < nMapWidth; ny++) {
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		}

		screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P'; // draws player position

		screen[nScreenWidth * nScreenHeight - 1] = '\0';// make the screen array a string
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}
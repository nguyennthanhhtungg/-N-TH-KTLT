#define _CRT_SECURE_NO_WARNINGS
#ifdef UNICODE
#define UNICODE
#endif

#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <thread>
#include <time.h>
#include <fstream>
using namespace std;

#pragma comment(lib, "winmm.lib")

//Hằng số
#define MAX_CAR 17
#define MAX_CAR_LENGTH 20
#define MAX_SPEED 3

//Biến toàn cục
POINT** X;  //Mảng chứa MAX_CAR xe
POINT Y;  //Đại diện người qua đường
int cnt = 0;  //Biến hỗ trợ trong quá trình tăng tốc xe di chuyển
int MOVING;  //Biến xác định hướng di chuyển của người
int SPEED;  //Tốc độ xe chạy (xem như level)
int HEIGH_CONSOLE = 20, WIDTH_CONSOLE = 80;  //Độ rộng và độ cao của màn hình Console
bool STATE;  //Trạng thái sống/chết của người qua đường
unsigned long times = 0;  //Thời gian chơi Game
int Level = 0;  //Đại diện cấp bậc  người chơi
char Ambulance[2][5];  //Mảng chứa các ký tự ghép thành xe cứu thương
int PauseTimes;  //Thời gian tạm dừng xe

/*Cố định kích thước màn hình Console*/
void FixConsoleWindow()
{
	HWND consoleWindow = GetConsoleWindow();
	LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
	style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
	SetWindowLong(consoleWindow, GWL_STYLE, style);
}

/*Xử lý tọa độ trên màn hình Console*/
void GotoXY(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

/*Hàm xử lý Color Game*/
void TextColor(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

/*Hàm xử lý vẽ đèn xanh đèn đỏ*/
void DrawLight(int color)
{
	for (int i = 2; i < HEIGH_CONSOLE; i += 2)
	{
		GotoXY(1, i);
		TextColor(color);
		cout << (char)(219);
	}
	TextColor(15);
}

/*Hàm khởi tạo dữ liệu mặc định ban đầu*/
void ResetData()
{
	MOVING = 'D';  //Ban đầu cho người di chuyển sang phải
	SPEED = 1;  //Tốc độ lúc đầu
	Y = { 18,19 };  //Vị trí lúc đầu của người
	//Tạo mảng xe chạy
	if (X == NULL)
	{
		X = new POINT*[MAX_CAR];
		for (int i = 0; i < MAX_CAR; i++)
			X[i] = new POINT[MAX_CAR_LENGTH];
		for (int i = 0; i < MAX_CAR; i++)
		{
			int temp = (rand() % (WIDTH_CONSOLE - MAX_CAR_LENGTH)) + 1;
			for (int j = 0; j < MAX_CAR_LENGTH; j++)
			{
				X[i][j].x = temp + j;
				X[i][j].y = 2 + i;
			}
		}
	}
}

/*Vẽ hình chữ nhật bao quanh làm phạm vi*/
void DrawBoard(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0)
{
	GotoXY(x, y);cout << 'X';
	for (int i = 1; i < width; i++)cout << 'X';
	cout << 'X';
	GotoXY(x, height + y);cout << 'X';
	for (int i = 1; i < width; i++)cout << 'X';
	cout << 'X';
	for (int i = y + 1; i < height + y; i++)
	{
		GotoXY(x, i); cout << 'X';
		GotoXY(x + width, i); cout << 'X';
	}
	GotoXY(curPosX, curPosY);
}

/*Vẽ xe cứu thương*/
void DrawAmbulance()
{
	Ambulance[0][0] = (char)(232);
	Ambulance[0][1] = (char)(196);
	Ambulance[0][2] = (char)(196);
	Ambulance[0][3] = (char)(196);
	Ambulance[0][4] = (char)(92);
	Ambulance[1][0] = (char)(192);
	Ambulance[1][1] = (char)(233);
	Ambulance[1][2] = (char)(196);
	Ambulance[1][3] = (char)(233);
	Ambulance[1][4] = (char)(217);
}

/*Xây dựng hàm StartGame*/
void StartGame()
{
	system("cls");
	ResetData();  //Khởi tạo dữ liệu gốc
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);  //Vẽ màn hình game
	STATE = true;  //Bắt đầu cho Thread chạy
	GotoXY(90, 1);
	printf_s("%d", Level);
}

/*Hàm ExitGame() và PauseGame()*/
void GabageCollect()  //Hàm dọn dẹp tài nguyên
{
	for (int i = 0; i < MAX_CAR; i++)
	{
		delete[] X[i];
	}
	delete[] X;
}

void ExitGame(HANDLE t)  //Hàm thoát Game
{
	system("cls");
	TerminateThread(t, 0);
	GabageCollect();
}

void PauseGame(HANDLE t)
{
	SuspendThread(t);
}

/*Hàm xử lý khi người đụng xe*/
void ProcessDead()
{
	STATE = 0;
	GotoXY(0, HEIGH_CONSOLE + 2);
	printf_s("Dead, type y to continue or anykey to exit");
}

/*Hàm xử lý khi người băng qua đường thành công*/
void ProcessFinish(POINT &p)
{
	SPEED == MAX_SPEED ? SPEED = 1 : SPEED++;
	p = { 18,19 };  //Vị trí lúc ban đầu của người
	MOVING = 'D';  //Ban đầu cho người di chuyển sang phải
}

/*Xử lý va chạm khi Y đã va chạm với các Y đã về trước đó*/
void XuLyVaChamY_Y(POINT Y, int *VT)
{

	if ((Y.y) == 1)
	{
		if (*(VT + Y.x) == 1)
		{
			int i = 0;
			while (i < WIDTH_CONSOLE + 1)
			{
				if (*(VT + i) == 1)
				{
					*(VT + i) = { NULL };
				}
				i++;
			}
			Level = 0;
			ProcessDead();
		}
		else
		{
			*(VT + Y.x) = 1;
			Level++;
			GotoXY(90, 1);
			printf_s("%d", Level);
		}
	}
}

void KeyPress()
{
	GotoXY(2, 26);
	printf_s("Developer: Nguyen Thanh Tung");
	GotoXY(2, 27);
	printf_s("MSSV: 1712884");
	GotoXY(82, 1);
	printf_s("Level: ");
	GotoXY(95, 1);
	printf_s("SPEED: ");
	GotoXY(103, 1);
	printf_s("%d", SPEED);
	GotoXY(82, 3);
	printf_s("Times: ");
	GotoXY(82, 5);
	printf_s("PRESS L TO SAVE GAME");
	GotoXY(82, 7);
	printf_s("PRESS T TO LOAD GAME");
	GotoXY(82, 9);
	printf_s("PRESS P TO PAUSE GAME");
	GotoXY(82, 11);
	printf_s("PRESS ESC TO EXIT GAME");
	GotoXY(82, 13);
	printf_s("PRESS:");
	GotoXY(97, 14);
	printf_s("W (GO UP)");
	GotoXY(97, 15);
	printf_s("%c",124);
	GotoXY(82, 16);
	printf_s("    A        %c %c %c         D", 196, 254, 196);
	GotoXY(82, 17);
	printf_s("(GO LEFT)      %c       (GO RIGHT)",124);
	GotoXY(97, 18);
	printf_s("S (GO DOWN)");
}

/*Hàm vẽ các toa xe*/
void DrawCars(const char* s)
{
	for (int i = 0; i < MAX_CAR; i++)
	{
		for (int j = 0; j < MAX_CAR_LENGTH; j++)
		{
			GotoXY(X[i][j].x, X[i][j].y);
			printf_s(s);
		}
	}
}

/*Hàm vẽ người qua đường*/
void DrawSticker(const POINT& p,const char* s)
{
	GotoXY(p.x, p.y);
	printf_s(s);
}

/*Hàm kiểm tra xem người qua đường có đụng xe không*/
bool IsImpact(const POINT& p, int d)
{
	if (d == 1 || d == 19) return false;
	for (int i = 0; i < MAX_CAR_LENGTH; i++)
	{
		if (p.x == X[d - 2][i].x && p.y == X[d - 2][i].y) return true;
	}
	return false;
}

/*Hàm xử lý di chuyển của xe*/
void MoveCars(int& PauseTimes)
{
	for (int i = 1; i < MAX_CAR; i += 2)
	{
		cnt = 0;
		do
		{
			cnt++;
			for (int j = 0; j < MAX_CAR_LENGTH - 1; j++)
			{
				X[i][j] = X[i][j + 1];
			}
			X[i][MAX_CAR_LENGTH - 1].x + 1 == WIDTH_CONSOLE ? X[i][MAX_CAR_LENGTH - 1].x = 1 : X[i][MAX_CAR_LENGTH - 1].x++;  //Kiểm tra xem xe có đụng màn hình không
		} while (cnt < SPEED);
	}
	PauseTimes++;
	if (PauseTimes >= 1 && PauseTimes <= 30)
	{
		DrawLight(10);
		for (int i = 0; i < MAX_CAR; i += 2)
		{
			cnt = 0;
			do
			{
				cnt++;
				for (int j = MAX_CAR_LENGTH - 1; j > 0; j--)
				{
					X[i][j] = X[i][j - 1];
				}
				X[i][0].x - 2 == 0 ? X[i][0].x = WIDTH_CONSOLE - 1 : X[i][0].x--;  //Kiểm tra xem xe có đụng màn hình không
			} while (cnt < SPEED);
		}
	}
	else
	{
		DrawLight(4);
	}
	if (PauseTimes == 50)
	{
		PauseTimes = 0;
	}
}

/*Hàm xử lý di chuyển của xe cứu thương*/
void MoveAmbulance(POINT p, int x, int y)
{
	for (int k = 1; k < WIDTH_CONSOLE - 5; k++)
	{
		GotoXY(k, p.y);
		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				cout << Ambulance[i][j];
			}
			GotoXY(k, p.y + 1);
			Sleep(30);
		}
		if (k == x - 6)
		{
			GotoXY(k, p.y - 2);
			printf_s("Help me.!!");
			Sleep(1000);
			GotoXY(k, p.y - 2);
			printf_s("          ");
		}
			GotoXY(k, p.y);
		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				cout << " ";
			}
			GotoXY(k, p.y + 1);
		}
	}
}

/*Hàm xóa xe (xóa có nghĩa là không vẽ)*/
void EraseCars()
{
	for (int i = 0; i < MAX_CAR; i += 2)
	{
		cnt = 0;
		do
		{
			GotoXY(X[i][MAX_CAR_LENGTH - 1 - cnt].x, X[i][MAX_CAR_LENGTH - 1 - cnt].y);
			printf_s(" ");
			cnt++;
		} while (cnt < SPEED);
	}
	for (int i = 1; i < MAX_CAR; i += 2)
	{
		cnt = 0;
		do
		{
			GotoXY(X[i][0 + cnt].x, X[i][0 + cnt].y);
			printf_s(" ");
			cnt++;
		} while (cnt < SPEED);
	}
}


/*Hàm xử lý di chuyển Y*/
void MoveRight()  //Di chuyển qua phải
{
	if (Y.x < WIDTH_CONSOLE - 1)
	{
		DrawSticker(Y, " ");
		Y.x++;
		DrawSticker(Y, "Y");
	}
}

void MoveLeft()  //Di chuyển qua trái
{
	if (Y.x > 1)
	{
		DrawSticker(Y, " ");
		Y.x--;
		DrawSticker(Y, "Y");
	}
}

void MoveDown()  //Di chuyển xuống
{
	if (Y.y < HEIGH_CONSOLE - 1)
	{
		DrawSticker(Y, " ");
		Y.y++;
		DrawSticker(Y, "Y");
	}
}

void MoveUp()  //Di chuyển lên
{
	if (Y.y > 1)
	{
		DrawSticker(Y, " ");
		Y.y--;
		DrawSticker(Y, "Y");
	}
}

/*Hàm xử lý về thoiè gian chơi Game*/
void SaveTimes()
{
	while (1)
	{
		GotoXY(90, 3);
		times++;
		printf_s("%d", times);
		Sleep(1000);
		GotoXY(90, 3);
		printf_s("  ");
	}
}

/*Hàm tạo hiệu ứng khi va chạm*/
void HieuUng(POINT p)
{
	system("cls");
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
	KeyPress();
	GotoXY(p.x, p.y);
	cout << "X";
	//const wchar_t Sound[14] = L"ambulance.wav";
	PlaySound("ambulance1.wav", NULL, SND_FILENAME);
	PlaySound("ambulance2.wav", NULL, SND_ASYNC);
	MoveAmbulance(Y, p.x, p.y);
}

/*Hàm xử lý tiểu trình khi băng qua đường*/
void SubThread()
{
	GotoXY(90, 1);
	printf_s("%d", Level);
	int DANHDAUVITRIY[81] = { NULL };
	int PauseTimes = 0;
	while (1)
	{
		if (STATE == true)
		{
			switch (MOVING)
			{
			case 'A':
				MoveLeft();
				break;
			case 'D':
				MoveRight();
				break;
			case 'W':
				MoveUp();
				break;
			case 'S':
				MoveDown();
				break;
			}
			MOVING = ' ';  //Tạm khóa không cho di chuyển, chờ nhận phím từ hàm main
			EraseCars();
			MoveCars(PauseTimes);
			DrawCars(".");
			//DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
			KeyPress();
			if (IsImpact(Y, Y.y))
			{
				int i = 0;
				while (i < WIDTH_CONSOLE + 1)
				{
					if (*(DANHDAUVITRIY + i) == 1)
					{
						*(DANHDAUVITRIY + i) = { NULL };
					}
					i++;
				}
				Level = 0;
				HieuUng(Y);
				ProcessDead();  //Kiểm tra xe có đụng không
			}
			XuLyVaChamY_Y(Y, DANHDAUVITRIY);
			if (Y.y == 1)
			{
				ProcessFinish(Y);  //Kiểm tra xem về đích chưa
			}
			Sleep(150);  //Hàm ngủ theo tốc độ SPEED
		}
	}
}

/*Làm ẩn con trỏ chuột trên màn hình Console*/
void Nocursortype()
{
	CONSOLE_CURSOR_INFO Info;
	Info.bVisible = FALSE;
	Info.dwSize = 20;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &Info);
}

/*Lưu thông tin Game khi đang chơi*/
void SaveInformation(fstream &fs)
{
	fs.seekg(0);
	fs << Level << endl;
	fs << SPEED << endl;
	fs << times << endl;
}
/*Hàm xử lý Lưu game*/
void SaveGame(fstream& fs, char *FileName)
{
	GotoXY(82, 20);
	cout << "----------SAVE GAME----------";
	GotoXY(85, 21);
	cout << "File location:";
	cin >> FileName;
	strcat(FileName, ".txt");
	fs.open(FileName, ios::out);
	if (fs.fail())
	{
		cout << "Khong the mo tap tin" << endl;
		_getch();
		return;
	}
	fs << Level << endl;
	fs << SPEED << endl;
	fs << times << endl;
}

/*Hàm xử lý lấy thông tin Game đã lưu*/
void GetInformation(fstream& fs, char *FileName)
{
	int Information;
	GotoXY(82, 20);
	printf_s("----------LOAD GAME----------");
	GotoXY(82, 21);
	printf_s("File location: ");
	cin >> FileName;
	strcat(FileName, ".txt");
	fs.open(FileName, ios::in);
	if (fs.fail())
	{
		GotoXY(82, 21);
		cout << "Khong the tim thay tap tin";
		_getch();
		return;
	}
	fs.seekg(0);
	fs >> Information;
	Level = Information;
	fs >> Information;
	SPEED = Information;
	fs >> Information;
	times = Information;
}

void main()
{
	fstream fs;
	int temp;
	char FileName[31];
	DrawAmbulance();
	FixConsoleWindow();
	srand(time(NULL));
	StartGame();
	Nocursortype();
	thread t1(SaveTimes);
	thread t2(SubThread);
	while (1)
	{
		temp = toupper(_getch());
		if (STATE == 1)
		{
			if (temp == 27)
			{
				ExitGame(t1.native_handle());
				ExitGame(t2.native_handle());
				return;
			}
			else if (temp == 'P')
			{
				PauseGame(t1.native_handle());
				PauseGame(t2.native_handle());
			}
			else if (temp == 'L')
			{
				PauseGame(t1.native_handle());
				PauseGame(t2.native_handle());
				SaveGame(fs,FileName);
			}
			else if (temp == 'T')
			{
				PauseGame(t1.native_handle());
				PauseGame(t2.native_handle());
				GetInformation(fs, FileName);
			}
			else
			{
				ResumeThread((HANDLE)t1.native_handle());
				ResumeThread((HANDLE)t2.native_handle());
				if (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S')
				{
					MOVING = temp;
				}
				if (!fs.fail())
				{
					SaveInformation(fs);
				}
			}
		}
		else
		{
			if (temp == 'Y')StartGame();
			else
			{
				ExitGame(t1.native_handle());
				ExitGame(t2.native_handle());
				return;
			}
		}
	}
}
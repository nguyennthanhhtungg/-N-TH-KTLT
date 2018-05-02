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
int ColorCar;  //Tô màu cho xe
int ColorBoard;  //Tô màu cho viền bảng
int ColorPerson;  //Tô màu cho nhân vật

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
void DrawBoard(int x, int y, int width, int height, int ColorBoard, int curPosX = 0, int curPosY = 0)
{
	if (ColorBoard != NULL)
	{
		TextColor(ColorBoard);
	}
	else
	{
		TextColor(15);
	}
	GotoXY(x, y); cout << '*';
	for (int i = 1; i < width; i++)cout << '*';
	cout << '*';
	GotoXY(x, height + y); cout << '*';
	for (int i = 1; i < width; i++)cout << '*';
	cout << '*';
	for (int i = y + 1; i < height + y; i++)
	{
		GotoXY(x, i); cout << '*';
		GotoXY(x + width, i); cout << '*';
	}
	GotoXY(curPosX, curPosY);
	TextColor(15);
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
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE, ColorBoard);  //Vẽ màn hình game
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

/*Hàm thêm chữ BACK*/
void DrawLetterBack()
{
	char GIAODIEN[2][4] = { NULL };

	GIAODIEN[0][0] = char(47);
	GIAODIEN[0][1] = char(205);
	GIAODIEN[0][2] = char(205);
	GIAODIEN[0][3] = char(205);
	GIAODIEN[1][0] = char(92);
	GIAODIEN[1][1] = char(205);
	GIAODIEN[1][2] = char(205);
	GIAODIEN[1][3] = char(205);
	for (int i = 0; i < 2; i++)
	{
		GotoXY(0, i + 27);
		for (int j = 0; j < 4; j++)
		{
			cout << GIAODIEN[i][j];
		}
		cout << endl;
	}
}

/*Hàm thêm ký tự nhập từ bàn phím*/
void KeyPress()
{
	GotoXY(86, 26);
	printf_s("Developer: Nguyen Thanh Tung");
	GotoXY(86, 27);
	printf_s("MSSV: 1712884");
	GotoXY(86, 28);
	printf_s("Class: 17CTT7");
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
	GotoXY(1, 26);
	printf_s("SETTING (PRESS R)");
	DrawLetterBack();
}

/*Hàm vẽ các toa xe*/
void DrawCars(const char* s,int ColorCar)
{
	if (ColorCar != NULL)
	{
		TextColor(ColorCar);
	}
	else
	{
		TextColor(15);
	}
	for (int i = 0; i < MAX_CAR; i++)
	{
		for (int j = 0; j < MAX_CAR_LENGTH; j++)
		{
			GotoXY(X[i][j].x, X[i][j].y);
			printf_s(s);
		}
	}
	TextColor(15);
}

/*Hàm vẽ người qua đường*/
void DrawSticker(const POINT& p,const char* s,int ColorPerson)
{
	if (ColorPerson != NULL)
	{
		TextColor(ColorPerson);
	}
	else
	{
		TextColor(15);
	}
	GotoXY(p.x, p.y);
	printf_s(s);
	TextColor(15);
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
		DrawSticker(Y, " ", ColorPerson);
		Y.x++;
		DrawSticker(Y, "Y", ColorPerson);
	}
}

void MoveLeft()  //Di chuyển qua trái
{
	if (Y.x > 1)
	{
		DrawSticker(Y, " ", ColorPerson);
		Y.x--;
		DrawSticker(Y, "Y", ColorPerson);
	}
}

void MoveDown()  //Di chuyển xuống
{
	if (Y.y < HEIGH_CONSOLE - 1)
	{
		DrawSticker(Y, " ", ColorPerson);
		Y.y++;
		DrawSticker(Y, "Y", ColorPerson);
	}
}

void MoveUp()  //Di chuyển lên
{
	if (Y.y > 1)
	{
		DrawSticker(Y, " ", ColorPerson);
		Y.y--;
		DrawSticker(Y, "Y", ColorPerson);
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
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE, ColorBoard);
	KeyPress();
	GotoXY(p.x, p.y);
	cout << "X";
	//const wchar_t Sound[14] = L"ambulance.wav";
	STATE = 0;
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
		PlaySound("Off Limits.wav", NULL, SND_ASYNC);
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
			DrawCars(".",ColorCar);
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

/*Hàm vẽ chữ Game*/
void DrawLetterGame()
{
	char GIAODIEN[5][50] = { NULL };

	//G
	GIAODIEN[0][0] = char(201);
	GIAODIEN[0][1] = char(205);
	GIAODIEN[0][2] = char(205);
	GIAODIEN[0][3] = char(205);
	GIAODIEN[0][4] = char(184);
	GIAODIEN[1][0] = char(186);
	GIAODIEN[2][0] = char(186);
	GIAODIEN[2][3] = char(201);
	GIAODIEN[2][4] = char(203);
	GIAODIEN[3][0] = char(186);
	GIAODIEN[3][4] = char(186);
	GIAODIEN[4][0] = char(200);
	GIAODIEN[4][1] = char(205);
	GIAODIEN[4][2] = char(205);
	GIAODIEN[4][3] = char(205);
	GIAODIEN[4][4] = char(202);

	//a
	GIAODIEN[2][6] = char(205);
	GIAODIEN[2][7] = char(205);
	GIAODIEN[2][8] = char(205);
	GIAODIEN[2][9] = char(183);
	GIAODIEN[3][6] = char(201);
	GIAODIEN[3][7] = char(205);
	GIAODIEN[3][8] = char(205);
	GIAODIEN[3][9] = char(186);
	GIAODIEN[4][6] = char(200);
	GIAODIEN[4][7] = char(205);
	GIAODIEN[4][8] = char(205);
	GIAODIEN[4][9] = char(202);


	//m
	GIAODIEN[2][11] = char(201);
	GIAODIEN[2][12] = char(205);
	GIAODIEN[2][13] = char(209);
	GIAODIEN[2][14] = char(205);
	GIAODIEN[2][15] = char(187);
	GIAODIEN[3][11] = char(186);
	GIAODIEN[3][13] = char(186);
	GIAODIEN[3][15] = char(186);
	GIAODIEN[4][11] = char(208);
	GIAODIEN[4][13] = char(208);
	GIAODIEN[4][15] = char(208);

	//e
	GIAODIEN[2][17] = char(201);
	GIAODIEN[2][18] = char(205);
	GIAODIEN[2][19] = char(205);
	GIAODIEN[2][20] = char(187);
	GIAODIEN[3][17] = char(204);
	GIAODIEN[3][18] = char(205);
	GIAODIEN[3][19] = char(205);
	GIAODIEN[3][20] = char(188);
	GIAODIEN[4][17] = char(200);
	GIAODIEN[4][18] = char(205);
	GIAODIEN[4][19] = char(205);
	GIAODIEN[4][20] = char(190);

	TextColor(4);
	for (int i = 0; i < 5; i++)
	{
		GotoXY(65, i + 2);
		for (int j = 0; j < 30; j++)
		{

			cout << GIAODIEN[i][j];
		}
		cout << endl;
	}
}

/*Hàm vẽ chữ STREET*/
void DrawLetterStreet()
{
	char GIAODIEN[5][40] = { NULL };

	//S
	GIAODIEN[0][0] = char(201);
	GIAODIEN[0][1] = char(205);
	GIAODIEN[0][2] = char(205);
	GIAODIEN[0][3] = char(205);
	GIAODIEN[0][4] = char(184);
	GIAODIEN[1][0] = char(186);
	GIAODIEN[2][0] = char(200);
	GIAODIEN[2][1] = char(205);
	GIAODIEN[2][2] = char(205);
	GIAODIEN[2][3] = char(205);
	GIAODIEN[2][4] = char(187);
	GIAODIEN[3][4] = char(186);
	GIAODIEN[4][0] = char(200);
	GIAODIEN[4][1] = char(205);
	GIAODIEN[4][2] = char(205);
	GIAODIEN[4][3] = char(205);
	GIAODIEN[4][4] = char(188);

	//T
	GIAODIEN[0][6] = char(201);
	GIAODIEN[0][7] = char(205);
	GIAODIEN[0][8] = char(205);
	GIAODIEN[0][9] = char(203);
	GIAODIEN[0][10] = char(205);
	GIAODIEN[0][11] = char(205);
	GIAODIEN[0][12] = char(187);
	GIAODIEN[1][9] = char(186);
	GIAODIEN[2][9] = char(186);
	GIAODIEN[3][9] = char(186);
	GIAODIEN[4][9] = char(208);

	//R
	GIAODIEN[0][14] = char(201);
	GIAODIEN[0][15] = char(205);
	GIAODIEN[0][16] = char(205);
	GIAODIEN[0][17] = char(205);
	GIAODIEN[0][18] = char(187);
	GIAODIEN[1][14] = char(186);
	GIAODIEN[1][18] = char(186);
	GIAODIEN[2][14] = char(204);
	GIAODIEN[2][15] = char(209);
	GIAODIEN[2][16] = char(205);
	GIAODIEN[2][17] = char(205);
	GIAODIEN[2][18] = char(188);
	GIAODIEN[3][14] = char(186);
	GIAODIEN[3][16] = char(92);
	GIAODIEN[3][17] = char(92);
	GIAODIEN[4][14] = char(211);
	GIAODIEN[4][17] = char(92);
	GIAODIEN[4][18] = char(92);

	//E
	GIAODIEN[0][20] = char(201);
	GIAODIEN[0][21] = char(205);
	GIAODIEN[0][22] = char(205);
	GIAODIEN[0][23] = char(205);
	GIAODIEN[1][20] = char(186);
	GIAODIEN[2][20] = char(204);
	GIAODIEN[2][21] = char(205);
	GIAODIEN[2][22] = char(205);
	GIAODIEN[2][23] = char(205);
	GIAODIEN[3][20] = char(186);
	GIAODIEN[4][20] = char(200);
	GIAODIEN[4][21] = char(205);
	GIAODIEN[4][22] = char(205);
	GIAODIEN[4][23] = char(205);

	//E
	GIAODIEN[0][25] = char(201);
	GIAODIEN[0][26] = char(205);
	GIAODIEN[0][27] = char(205);
	GIAODIEN[0][28] = char(205);
	GIAODIEN[1][25] = char(186);
	GIAODIEN[2][25] = char(204);
	GIAODIEN[2][26] = char(205);
	GIAODIEN[2][27] = char(205);
	GIAODIEN[2][28] = char(205);
	GIAODIEN[3][25] = char(186);
	GIAODIEN[4][25] = char(200);
	GIAODIEN[4][26] = char(205);
	GIAODIEN[4][27] = char(205);
	GIAODIEN[4][28] = char(205);

	//T
	GIAODIEN[0][30] = char(201);
	GIAODIEN[0][31] = char(205);
	GIAODIEN[0][32] = char(205);
	GIAODIEN[0][33] = char(203);
	GIAODIEN[0][34] = char(205);
	GIAODIEN[0][35] = char(205);
	GIAODIEN[0][36] = char(187);
	GIAODIEN[1][33] = char(186);
	GIAODIEN[2][33] = char(186);
	GIAODIEN[3][33] = char(186);
	GIAODIEN[4][33] = char(208);

	TextColor(10);
	for (int i = 0; i < 5; i++)
	{
		GotoXY(55, i + 8);
		for (int j = 0; j < 40; j++)
		{
			cout << GIAODIEN[i][j];
		}
		cout << endl;
	}
}

/*Hàm vẽ chữ CROSSING*/
void DrawLetterCrosing()
{

	char GIAODIEN[5][48] = { NULL };

	//C
	GIAODIEN[0][0] = char(201);
	GIAODIEN[0][1] = char(205);
	GIAODIEN[0][2] = char(205);
	GIAODIEN[0][3] = char(205);
	GIAODIEN[0][4] = char(205);
	GIAODIEN[0][4] = char(187);
	GIAODIEN[1][0] = char(186);
	GIAODIEN[2][0] = char(186);
	GIAODIEN[3][0] = char(186);
	GIAODIEN[4][0] = char(200);
	GIAODIEN[4][1] = char(205);
	GIAODIEN[4][2] = char(205);
	GIAODIEN[4][3] = char(205);
	GIAODIEN[4][4] = char(188);

	//R
	GIAODIEN[0][6] = char(201);
	GIAODIEN[0][7] = char(205);
	GIAODIEN[0][8] = char(205);
	GIAODIEN[0][9] = char(205);
	GIAODIEN[0][10] = char(187);
	GIAODIEN[1][6] = char(186);
	GIAODIEN[1][10] = char(186);
	GIAODIEN[2][6] = char(204);
	GIAODIEN[2][7] = char(209);
	GIAODIEN[2][8] = char(205);
	GIAODIEN[2][9] = char(205);
	GIAODIEN[2][10] = char(188);
	GIAODIEN[3][6] = char(186);
	GIAODIEN[3][8] = char(92);
	GIAODIEN[3][9] = char(92);
	GIAODIEN[4][6] = char(211);
	GIAODIEN[4][9] = char(92);
	GIAODIEN[4][10] = char(92);

	//O
	GIAODIEN[0][12] = char(201);
	GIAODIEN[0][13] = char(205);
	GIAODIEN[0][14] = char(205);
	GIAODIEN[0][15] = char(205);
	GIAODIEN[0][16] = char(205);
	GIAODIEN[0][16] = char(187);
	GIAODIEN[1][12] = char(186);
	GIAODIEN[2][12] = char(186);
	GIAODIEN[3][12] = char(186);
	GIAODIEN[4][12] = char(200);
	GIAODIEN[4][13] = char(205);
	GIAODIEN[4][14] = char(205);
	GIAODIEN[4][15] = char(205);
	GIAODIEN[4][16] = char(188);
	GIAODIEN[1][16] = char(186);
	GIAODIEN[2][16] = char(186);
	GIAODIEN[3][16] = char(186);

	//S
	GIAODIEN[0][18] = char(201);
	GIAODIEN[0][19] = char(205);
	GIAODIEN[0][20] = char(205);
	GIAODIEN[0][21] = char(205);
	GIAODIEN[0][22] = char(184);
	GIAODIEN[1][18] = char(186);
	GIAODIEN[2][18] = char(200);
	GIAODIEN[2][19] = char(205);
	GIAODIEN[2][20] = char(205);
	GIAODIEN[2][21] = char(205);
	GIAODIEN[2][22] = char(187);
	GIAODIEN[3][22] = char(186);
	GIAODIEN[4][18] = char(200);
	GIAODIEN[4][19] = char(205);
	GIAODIEN[4][20] = char(205);
	GIAODIEN[4][21] = char(205);
	GIAODIEN[4][22] = char(188);

	//S
	GIAODIEN[0][24] = char(201);
	GIAODIEN[0][25] = char(205);
	GIAODIEN[0][26] = char(205);
	GIAODIEN[0][27] = char(205);
	GIAODIEN[0][28] = char(184);
	GIAODIEN[1][24] = char(186);
	GIAODIEN[2][24] = char(200);
	GIAODIEN[2][25] = char(205);
	GIAODIEN[2][26] = char(205);
	GIAODIEN[2][27] = char(205);
	GIAODIEN[2][28] = char(187);
	GIAODIEN[3][28] = char(186);
	GIAODIEN[4][24] = char(200);
	GIAODIEN[4][25] = char(205);
	GIAODIEN[4][26] = char(205);
	GIAODIEN[4][27] = char(205);
	GIAODIEN[4][28] = char(188);

	//I
	GIAODIEN[0][30] = char(210);
	GIAODIEN[1][30] = char(186);
	GIAODIEN[2][30] = char(186);
	GIAODIEN[3][30] = char(186);
	GIAODIEN[4][30] = char(208);

	//N
	GIAODIEN[0][32] = char(210);
	GIAODIEN[1][32] = char(186);
	GIAODIEN[2][32] = char(186);
	GIAODIEN[3][32] = char(186);
	GIAODIEN[4][32] = char(208);
	GIAODIEN[0][33] = char(92);
	GIAODIEN[1][33] = char(92);
	GIAODIEN[1][34] = char(92);
	GIAODIEN[2][34] = char(92);
	GIAODIEN[2][35] = char(92);
	GIAODIEN[3][35] = char(92);
	GIAODIEN[3][36] = char(92);
	GIAODIEN[4][36] = char(92);
	GIAODIEN[0][37] = char(210);
	GIAODIEN[1][37] = char(186);
	GIAODIEN[2][37] = char(186);
	GIAODIEN[3][37] = char(186);
	GIAODIEN[4][37] = char(208);

	//G
	GIAODIEN[0][39] = char(201);
	GIAODIEN[0][40] = char(205);
	GIAODIEN[0][41] = char(205);
	GIAODIEN[0][42] = char(205);
	GIAODIEN[0][43] = char(184);
	GIAODIEN[1][39] = char(186);
	GIAODIEN[2][39] = char(186);
	GIAODIEN[2][42] = char(201);
	GIAODIEN[2][43] = char(203);
	GIAODIEN[3][39] = char(186);
	GIAODIEN[3][43] = char(186);
	GIAODIEN[4][39] = char(200);
	GIAODIEN[4][40] = char(205);
	GIAODIEN[4][41] = char(205);
	GIAODIEN[4][42] = char(205);
	GIAODIEN[4][43] = char(202);

	TextColor(13);
	for (int i = 0; i < 5; i++)
	{
		GotoXY(70, i + 14);
		for (int j = 0; j < 48; j++)
		{

			cout << GIAODIEN[i][j];
		}
		cout << endl;
	}
}

/*Hàm vẽ viền Giao diện*/
void DrawHem()
{
	TextColor(4);
	for (int i = 0; i < 120; i++)
	{
		cout << char(177);
	}

	for (int i = 0; i < 30; i++)
	{
		GotoXY(0, i);
		cout << char(177);
		cout << char(177);
		GotoXY(118, i);
		cout << char(177);
		cout << char(177);
	}

	for (int i = 1; i < 119; i++)
	{
		GotoXY(i, 29);
		cout << char(177);
	}
}

/*Hàm xử lý MENU*/
void DrawChoose()
{
	TextColor(4);
	for (int i = 7; i < 15; i++)
	{
		GotoXY(13, i);
		cout << char(178);
		GotoXY(29, i);
		cout << char(178);
	}
	GotoXY(16, 6);
	cout << "_-_-MENU-_-_";
	GotoXY(15, 7);
	cout << "<1>_NEW GAME";
	GotoXY(15, 9);
	cout << "<2>_LOAD GAME";
	GotoXY(15, 11);
	cout << "<3>_SETTINGS";
	GotoXY(15, 13);
	cout << "<4>_EXIT GAME";
	TextColor(15);
}

/*Hàm thêm nhạc nền*/
void BackgroundMusic()
{
	PlaySound("Off Limits.wav", NULL, SND_ASYNC);
}

/*Hàm xử lý thông tin sinh viên*/
void DrawInformationStudent(int &Choose)
{
	TextColor(1);
	GotoXY(74, 24);
	cout << "FULL NAME: NGUYEN THANH TUNG";
	GotoXY(74, 25);
	cout << "MSSV: 1712884";
	GotoXY(74, 26);
	cout << "17CTT7 UNIVERSITY OF SCIENCE";
	GotoXY(74, 27);
	cout << "LECTURER: TRUONG TOAN THINH";
	BackgroundMusic();
	cin >> Choose;
	if (Choose != NULL)
	{
		PlaySound(NULL, NULL, SND_PURGE);
	}
}

/*Hàm lựa chọn Setting*/
void DrawSetting()
{
	for (int i = 0; i < 256; i++)
	{
		TextColor(i);
		printf("  Press: %d  ", i);
		TextColor(15);
		printf(": %d", i);
	}
	TextColor(15);
	printf_s("\n");
	printf_s("Color Car: \n");
	printf_s("Color Board: \n");
	printf_s("Color Person: \n");
	TextColor(15);
}

/*Hàm lựa chọn khi Press R*/
void PressSetting()
{
	GotoXY(2, 4);
	for (int i = 0; i < 62; i++)
	{
		TextColor(i);
		cout << char(219);
	}
	TextColor(10);
	GotoXY(4, 6);
	printf_s("Color Car: \n");
	GotoXY(4, 7);
	printf_s("Color Board: \n");
	GotoXY(4, 8);
	printf_s("Color Person: \n");
	TextColor(15);
}

/*Hàm vẽ nút Return*/
void DrawLetterReturn()
{
	char GIAODIEN[2][4] = { NULL };


	GIAODIEN[0][0] = char(205);
	GIAODIEN[0][1] = char(205);
	GIAODIEN[0][2] = char(205);
	GIAODIEN[0][3] = char(92);
	GIAODIEN[1][0] = char(205);
	GIAODIEN[1][1] = char(205);
	GIAODIEN[1][2] = char(205);
	GIAODIEN[1][3] = char(47);
	for (int i = 0; i < 2; i++)
	{
		GotoXY(113, i + 27);
		for (int j = 0; j < 4; j++)
		{
			cout << GIAODIEN[i][j];
		}
	}
	GotoXY(100, 26);
	printf("RETURN (PRESS R)");

}
void main()
{
	int Choose;
	fstream fs;
	int temp;
	char FileName[31];
	DrawAmbulance();
	FixConsoleWindow();
	Nocursortype();
	DrawHem();
	DrawLetterGame();
	DrawLetterStreet();
	DrawLetterCrosing();
	DrawChoose();
	DrawInformationStudent(Choose);
	while (1)
	{
		if (Choose == 1)
		{
			srand(time(NULL));
			StartGame();
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
						SaveGame(fs, FileName);
					}
					else if (temp == 'T')
					{
						PauseGame(t1.native_handle());
						PauseGame(t2.native_handle());
						GetInformation(fs, FileName);
					}
					else if (temp == 'R')
					{
						/*ExitGame(t2.native_handle());*/
						PauseGame(t1.native_handle());
						PauseGame(t2.native_handle());
						system("cls");
						DrawHem();
						DrawLetterGame();
						DrawLetterStreet();
						DrawLetterCrosing();
						PressSetting();
						DrawLetterReturn();
						GotoXY(15, 6);
						cin >> ColorCar;
						GotoXY(17, 7);
						cin >> ColorBoard;
						GotoXY(18, 8);
						cin >> ColorPerson;
						char press;
						cin >> press;
						if (press == 'r' || press == 'R')
						{
							system("cls");
							StartGame();
							continue;
						}
					/*	_getch();*/
				/*		ExitGame(t1.native_handle());*/
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
		else if (Choose == 2)
		{
			GetInformation(fs, FileName);
			Choose = 1; continue;
		}
		else if (Choose == 3)
		{
			system("cls");
			DrawSetting();
			GotoXY(11, 39);
			cin >> ColorCar;
			GotoXY(13, 40);
			cin >> ColorBoard;
			GotoXY(14, 41);
			cin >> ColorPerson;
			system("cls");
			DrawHem();
			DrawLetterGame();
			DrawLetterStreet();
			DrawLetterCrosing();
			DrawChoose();
			DrawInformationStudent(Choose); continue;
		}
		else if (Choose == 4)
		{
			break;
		}
		else
		{
			return;
		}
	}
}
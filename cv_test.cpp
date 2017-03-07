#include <iostream>
#include "cv.h"
#include "highgui.h"

using namespace std;
using namespace cv;

int recordHighNote[30][4] = { 0, }; // 높은음자리표의 위치,색을 저장하는 배열 recordArray[i][0]:몇번째인지 recordArray[i][1]:x좌표 recordArray[i][2]:y좌표 recordArray[i][3]:RGB값(색상)

Mat toBinary(Mat template_img) //이미지를 불러오고 gray화 -> 이진화 하는 함수
{
	Mat gray_note;
	cvtColor(template_img, gray_note, CV_BGR2GRAY);
	imshow("toGray image", gray_note); // gray화 확인용

	Mat binary_note;
	adaptiveThreshold(~gray_note, binary_note, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -10);
	imshow("toBinary grayimage", binary_note); // 이진화 확인용
	return binary_note;
}

Mat deleteLine(Mat musical_note) // 오선 제거 하는 함수
{

	int verticalsize = musical_note.rows / 100;
	Mat verticalStructure = getStructuringElement(MORPH_RECT, Size(1, verticalsize));
	erode(musical_note, musical_note, verticalStructure, Point(-1, -1)); //찾은 오선부분 제거
	dilate(musical_note, musical_note, verticalStructure, Point(-1, -1)); //오선 부분 지워진곳 매끄럽게
	imshow("delete 5line", musical_note); //오선제거 확인용

	Mat delete_5line = musical_note;
	return delete_5line;
}

int(*findHighNote(Mat delete_5line, Mat binary_high_note))[4] // 높은음자리표 찾아 그 좌표와 갯수, RGB값이 저장된 배열을 리턴하는 함수
{
	Mat img_result(delete_5line.rows - binary_high_note.rows + 1, delete_5line.cols - binary_high_note.cols + 1, CV_32FC1);
	//matchTemplate(gray_ref, gray_tpl, res, CV_TM_CCOEFF_NORMED);
	matchTemplate(delete_5line, binary_high_note, img_result, CV_TM_CCOEFF_NORMED);
	threshold(img_result, img_result, 0.4, 1., CV_THRESH_TOZERO); // 상관계수가 0.4 이상인 곳

	int numOfHighNote = 0; // 템플릿 매칭 반복문을 돌면서 높은 음자리수를 몇개를 찾았는지 체크하기 위한 변수


	int digit = 0; // recordArray 배열을 탐색하기 위한 변수

	int color = 255; //초기 색 RGB값
	while (true)
	{
		double minval, maxval, threshold = 0.01;
		Point minloc, maxloc;
		minMaxLoc(img_result, &minval, &maxval, &minloc, &maxloc);

		if (maxval >= threshold) // 높은 음자리표로 유추되는 이미지의 maxval값이 임계점보다 크다면 
		{
			rectangle(      // 사각형으로 라벨링을 한다.
				delete_5line,
				maxloc,
				Point(maxloc.x + binary_high_note.cols, maxloc.y + binary_high_note.rows),
				CV_RGB(color, color, color), 2

				);

			recordHighNote[digit][0] = numOfHighNote;    //몇번째 높은 음자리표인지 배열에 저장
			recordHighNote[digit][1] = maxloc.x; //x좌표 배열에 저장
			recordHighNote[digit][2] = maxloc.y; //y좌표 배열에 저장
			recordHighNote[digit][3] = color; // RGB값 저장

			color = color - 80; // 템플릿매칭을 하면서 색의 값들을 구분하기 위해 RGB값을 줄여준다.

			//putText(vertical_ref, to_string(count), Point(maxloc.x + 20, maxloc.y + 20), FONT_HERSHEY_SIMPLEX, 1, Scalar(255), 2);
			//putText : 라벨링한거에 번호 매기는것

			floodFill(img_result, maxloc, Scalar(0), 0, Scalar(.1), Scalar(1.));



			digit++;
			numOfHighNote++;

		}
		else
			break;
	}

	for (int i = 0; i < numOfHighNote; i++) //  높은 음자리표 위치 및 RGB값 확인
	{
		cout << recordHighNote[i][0] << "번째 높은음자리표" << " " << "x위치값 = " << " " << recordHighNote[i][1] << " " << "y위치값 = " << recordHighNote[i][2] << " " << "RGB값 = " << recordHighNote[i][3] << endl;
	}

	Mat find_high_note = delete_5line;

	imshow("find high note", find_high_note); //높은 음자리표 템플릿 매칭 확인용

	return recordHighNote; // 높은 음자리표 위치 및 RGB값 리턴
}

Mat findNote(Mat delete_5line, Mat binary_note, int(*recordHighNote)[4],double noteThresHold) //오선제거된 악보와 , 템플릿매칭할 음표 , 높은 음자리표 좌표를 기준으로 마디 단위로 음표 찾기
{
	Mat img_result(delete_5line.rows - binary_note.rows + 1, delete_5line.cols - binary_note.cols + 1, CV_32FC1);
	//imshow("find note - delete_5line", delete_5line); //매칭 확인용
	//imshow("find note - binary_note", binary_note); //매칭 확인용
	//imshow("find note - img_result", img_result);

	matchTemplate(delete_5line, binary_note, img_result, CV_TM_CCOEFF_NORMED);

	

	threshold(img_result, img_result, noteThresHold, 1., CV_THRESH_TOZERO); // 상관계수가 0.75 이상인 곳



	int rgbColor = 255; //초기 색 RGB값
	while (true)
	{
		double minval, maxval, threshold = 0.01;
		Point minloc, maxloc;
		minMaxLoc(img_result, &minval, &maxval, &minloc, &maxloc);

		if (maxval >= threshold) // 찾고자 하는 음표로 유추되는 이미지의 maxval값이 임계점보다 크다면 
		{
			if (maxloc.y + 10 > recordHighNote[0][2] - 10) //첫번째 높은음자리표 구간이면
			{
				rgbColor = recordHighNote[0][3]; //첫번째 높은음자리표의 RGB값을
			}
			if (maxloc.y + 10 > recordHighNote[1][2] - 10) //두번째 높은음자리표 구간이면
			{
				rgbColor = recordHighNote[1][3];//두번째 높은음자리표 구간이면
			}
			if (maxloc.y + 10 > recordHighNote[2][2] - 10) //세번째 높은음자리표 구간이면
			{
				rgbColor = recordHighNote[2][3]; //세번째 높은음자리표 구간이면
			}
			rectangle(      // 사각형으로 라벨링을 한다.
				delete_5line,
				maxloc,
				Point(maxloc.x + binary_note.cols, maxloc.y + binary_note.rows),
				CV_RGB(rgbColor, rgbColor, rgbColor), 2

				);

			floodFill(img_result, maxloc, Scalar(0), 0, Scalar(.1), Scalar(1.));


		}
		else
			break;
	}

	imshow("find note", delete_5line); //오선제거 확인용
	return delete_5line;

}
int main()
{

	Mat musical_note = imread("bear.jpg"); //악보
	Mat high_note = imread("highnote.jpg");//높은 음자리표
	Mat tpl_8note = imread("8note_sample.jpg"); //8분음표
	Mat tpl_4note = imread("4note_sample.jpg"); //4분음표
	Mat tpl_2note = imread("2note_sample.jpg"); //2분음표

	Mat binary_note = toBinary(musical_note); //악보 이진화
	Mat binary_high_note = toBinary(high_note); //높은 음자리표 이진화
	Mat binary_8note = toBinary(tpl_8note); //8분음표 이진화
	Mat binary_4note = toBinary(tpl_4note); //4분음표 이진화
	Mat binary_2note = toBinary(tpl_2note); //2분음표 이진화


	Mat delete_5line = deleteLine(binary_note); // 이진화 한 악보 오선 제거 

	int(*positionHighNote)[4] = findHighNote(delete_5line, binary_high_note);

	//오선 삭제된 악보와 찾고자하는 템플릿 음표와 높은음자리표의 좌표를 이용해 음표를 찾는다.
	Mat find_4note = findNote(delete_5line, binary_4note, positionHighNote,0.75);
	Mat find_8note = findNote(find_4note, binary_8note, positionHighNote,0.75);
	Mat find_2note = findNote(find_8note, binary_2note, positionHighNote,0.51);


	waitKey(0);

	return 0;

}
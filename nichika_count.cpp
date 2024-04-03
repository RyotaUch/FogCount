// 2値化した画像の点の個数を求めるコード

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <chrono>

std::vector<int> image; //1次元ベクトル(std::vector<int>)で構成 　　　std::vector:一次元配列を表す。
std::vector<int> labeledImage;
int label_number = 0;

void nichika(cv::Mat& img, cv::Mat& output, int thresh){
        
    for (int y = 0; y < output.rows; y++){    //画像の各ピクセルに対して、テーブルを使用して輝度値を変換する。
        for (int x = 0; x < output.cols; x++){
            
            if (output.at<uchar>(y, x) < thresh){  //最小値未満の輝度値はすべて0になる。
                output.at<uchar>(y, x) = 0;
            }else{
                output.at<uchar>(y, x) = 255;
            }
        }
    }
}

void Labeling(cv::Mat& output, int j, int i, int label_number, int index, std::vector<int>& labeledImage) {

    labeledImage[index] = label_number;

    for (int k = -1; k <= 1; k++) {
        for (int l = -1; l <= 1; l++) {
            int newCol = j + l;
            int newRow = i + k;
            int newindex = newRow * output.cols + newCol;

            if (newRow >= 0 && newRow < output.rows && newCol >= 0 && newCol < output.cols && labeledImage[newindex] == 0 && output.at<uchar>(newRow, newCol) != 0) {
                Labeling(output, newCol, newRow, label_number, newindex, labeledImage);
            }
        }
    }
}

void drawColors(cv::Mat& output, cv::Mat& coloredImage){

    // 各ラベルに異なる色を使って塗りつぶす

    std::vector<cv::Vec3b> labelColors;

    labelColors.reserve(label_number);

    for (int i = 0; i <= label_number; i++){
        // ランダムに色を作る。
        cv::Vec3b color(rand() % 256, rand() % 256, rand() % 256); 
        labelColors[i] = color;
    }

    for (int i = 0; i < output.rows; i++) {
        for (int j = 0; j < output.cols; j++) {
            int label_number = labeledImage[i * output.cols + j];

            if (label_number > 0 && label_number <= label_number) {  // ラベルが割り当てられている場合
                coloredImage.at<cv::Vec3b>(i, j) = labelColors[label_number - 1]; // cv::<Vec3b> : 3つの要素を持つベクトル color[0]: B（青）の成分 , color[1]: G（緑）の成分 , color[2]: R（赤）の成分
            }
        }
    }
}




int main(){

    auto start_time = std::chrono::high_resolution_clock::now();

    cv::Mat img = cv::imread("thresh1.png", cv::IMREAD_GRAYSCALE);

    if (img.empty()) {
        std::cerr << "Failed to open the img" << std::endl;
        return -1;
    }

    cv::Mat output = img.clone();

    nichika(img, output, 30);

    int height = output.rows;
    int width = output.cols;

    cv::Mat coloredImage = cv::Mat::zeros(height, width, CV_8UC3);

    // ラベルの初期化(初期値は全て0) 
    int index = 0;     
    labeledImage.resize(height*width, 0);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            index = i*width + j;  // 各ピクセルの座標

            // imageという配列があり、その中(imageの中)のindex番目の要素の値(i,j)の値を取り出して、それをvalueという変数に格納する。
            int value = output.at<uchar>(i, j);
   
            if (value == 0){  
                labeledImage[index] = -1;  // 配列に注意?　index番目のlabelを-1と定める。
                continue;  // continue:for文に帰る。
            }

            if (value == 255 && labeledImage[index] == 0){  // ピクセルの値が255でかつまだラベリングされていない場合
                label_number++;
                Labeling(output, j, i, label_number, index, labeledImage); // Labeling関数の呼び出し
            }
        }
    }

/*
    // 各ラベルに対して条件を満たさない場合、ピクセルを0に設定
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            index = i*width + j;
            // int label_number = labeledImage[index];

            if (label_number > 0 && label_number <= label_number && pixelcounts[label_number - 1] < 10) {
                labeledImage[index] = 0;
                output.at<uchar>(i, j) = 0;
            }
        }
    }
*/

    drawColors(img, coloredImage);

    std::cout << "カウント数:" << label_number << std::endl;

    cv:imshow("Original", img);
    cv::imwrite("Original.tiff", output);
    cv::imshow("labelImage", output);
    cv::imshow("colored Image", coloredImage);
    cv::imwrite("Colored.tiff", coloredImage);

    auto end_time = std::chrono::high_resolution_clock::now();

    // 処理にかかった時間を計算
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 処理にかかった時間を表示
    std::cout << "time:" << duration.count() << "m seconds" << std::endl;

    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}


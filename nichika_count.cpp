// 2�l�������摜�̓_�̌������߂�R�[�h

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <chrono>

std::vector<int> image; //1�����x�N�g��(std::vector<int>)�ō\�� �@�@�@std::vector:�ꎟ���z���\���B
std::vector<int> labeledImage;
int label_number = 0;

void nichika(cv::Mat& img, cv::Mat& output, int thresh){
        
    for (int y = 0; y < output.rows; y++){    //�摜�̊e�s�N�Z���ɑ΂��āA�e�[�u�����g�p���ċP�x�l��ϊ�����B
        for (int x = 0; x < output.cols; x++){
            
            if (output.at<uchar>(y, x) < thresh){  //�ŏ��l�����̋P�x�l�͂��ׂ�0�ɂȂ�B
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

    // �e���x���ɈقȂ�F���g���ēh��Ԃ�

    std::vector<cv::Vec3b> labelColors;

    labelColors.reserve(label_number);

    for (int i = 0; i <= label_number; i++){
        // �����_���ɐF�����B
        cv::Vec3b color(rand() % 256, rand() % 256, rand() % 256); 
        labelColors[i] = color;
    }

    for (int i = 0; i < output.rows; i++) {
        for (int j = 0; j < output.cols; j++) {
            int label_number = labeledImage[i * output.cols + j];

            if (label_number > 0 && label_number <= label_number) {  // ���x�������蓖�Ă��Ă���ꍇ
                coloredImage.at<cv::Vec3b>(i, j) = labelColors[label_number - 1]; // cv::<Vec3b> : 3�̗v�f�����x�N�g�� color[0]: B�i�j�̐��� , color[1]: G�i�΁j�̐��� , color[2]: R�i�ԁj�̐���
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

    // ���x���̏�����(�����l�͑S��0) 
    int index = 0;     
    labeledImage.resize(height*width, 0);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            index = i*width + j;  // �e�s�N�Z���̍��W

            // image�Ƃ����z�񂪂���A���̒�(image�̒�)��index�Ԗڂ̗v�f�̒l(i,j)�̒l�����o���āA�����value�Ƃ����ϐ��Ɋi�[����B
            int value = output.at<uchar>(i, j);
   
            if (value == 0){  
                labeledImage[index] = -1;  // �z��ɒ���?�@index�Ԗڂ�label��-1�ƒ�߂�B
                continue;  // continue:for���ɋA��B
            }

            if (value == 255 && labeledImage[index] == 0){  // �s�N�Z���̒l��255�ł��܂����x�����O����Ă��Ȃ��ꍇ
                label_number++;
                Labeling(output, j, i, label_number, index, labeledImage); // Labeling�֐��̌Ăяo��
            }
        }
    }

/*
    // �e���x���ɑ΂��ď����𖞂����Ȃ��ꍇ�A�s�N�Z����0�ɐݒ�
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

    std::cout << "�J�E���g��:" << label_number << std::endl;

    cv:imshow("Original", img);
    cv::imwrite("Original.tiff", output);
    cv::imshow("labelImage", output);
    cv::imshow("colored Image", coloredImage);
    cv::imwrite("Colored.tiff", coloredImage);

    auto end_time = std::chrono::high_resolution_clock::now();

    // �����ɂ����������Ԃ��v�Z
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // �����ɂ����������Ԃ�\��
    std::cout << "time:" << duration.count() << "m seconds" << std::endl;

    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}


//全画像に適用されるメジアンフィルタのコード

#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <algorithm>

void medianFilter(cv::Mat& img1, cv::Mat& output, int KernelSize) {

    // メジアンフィルタを適用
    for (int y = 0; y < img1.rows; y+=1) {
        for (int x = 0; x < img1.cols; x+=1) {
            // std::cout << x << std::endl;

            int index = 0;

            // カーネル内の値を取得
            std::vector<uchar> values;

            values.resize(KernelSize * KernelSize);
            // values.reserve(KernelSize * KernelSize);

            //各ピクセルの周囲にあるカーネル内の値の取得
            for (int i = -KernelSize / 2; i <= KernelSize / 2; i++) {
                for (int j = -KernelSize / 2; j <= KernelSize / 2; j++) {
                    int yy = std::max(0, std::min(img1.rows - 1, y + i));  //img1.rows - 1、img.cols - 1:画像の行数・列数を超えないようにするためにこのような設定にしている。
                    int xx = std::max(0, std::min(img1.cols - 1, x + j));
                    // values.push_back(output.at<uchar>(yy, xx));
                    values[index] = output.at<uchar>(yy, xx);
                    index++;
                }
            }

            // sort(values.begin(), values.end());

            // 取得した値を昇順にデータの並び替えをする。(ソートする)
            std::nth_element(values.begin(), values.begin() + values.size() / 2, values.end());  // valuesベクトルにカーネル内のピクセル値がソートされた状態になっている。

            // 中央値を新値として設定
            output.at<uchar>(y, x) = values[values.size() / 2 + values.size() / 5] ;  // valuesの[]番目の値に相当するピクセル値がoutputの画像の(y, x)に代入される。
        }
    }
}

void subtractMatrices(cv::Mat& img1, cv::Mat& img2, cv::Mat resultImage) {

    for (int i = 0; i < img1.rows; ++i) {
        for (int j = 0; j < img1.cols; ++j) {
            resultImage.at<uchar>(i, j) = 2.5 * std::max(0, (img2.at<uchar>(i, j) - img1.at<uchar>(i, j)));
        }
    }
}

int main() {

    auto start_time = std::chrono::high_resolution_clock::now();
    
    cv::Mat img1 = cv::imread("30115512.tiff", cv::IMREAD_GRAYSCALE);
    
    cv::Mat output = img1.clone();  // 元の画像(img1)を変更することなく、そのコピーを新しい変数outputに格納する。 後に、outputが画像処理される。

    if (img1.empty()) {
        std::cerr << "No image" << std::endl;
        return -1;
    }

    // メジアンフィルタを適用
    medianFilter(img1, output, 3);  // Kernelsizeは3以上の奇数

    cv::imshow("InputImage", img1);
    cv::imshow("MedianFilterImage", output);
    cv::imwrite("median.tiff", output);

    cv::Mat img2 = cv::imread("median.tiff", cv::IMREAD_GRAYSCALE);

    cv::Mat resultImage(img1.rows, img1.cols, img1.type());

    subtractMatrices(img1, img2, resultImage);

    cv::imshow("SubtractImage", resultImage);
    cv::imwrite("shingazouImage.tiff", resultImage);

    auto end_time = std::chrono::high_resolution_clock::now();

    // 処理にかかった時間を計算
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 結果を表示
    std::cout << "time" << duration.count() << "millisecond" << std::endl;

    cv::waitKey(0);
    cv::destroyAllWindows();

}



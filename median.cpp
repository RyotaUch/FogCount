#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>

void medianFilter(cv::Mat& img, cv::Mat& output, int KernelSize) {

    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {

            int index = 0;

            // カーネル内の値を取得
            std::vector<uchar> values;

            values.resize(KernelSize * KernelSize);

            // 各ピクセルの周囲にあるカーネル内の値の取得
            for (int i = -KernelSize / 2; i <= KernelSize / 2; i++) {
                for (int j = -KernelSize / 2; j <= KernelSize / 2; j++) {
                    // int zz = std::max(0, std::min(image.size[0] - 1, z + k));
                    int yy = std::max(0, std::min(img.rows - 1, y + i));
                    int xx = std::max(0, std::min(img.cols - 1, x + j));
                    values[index] = output.at<uchar>(yy, xx);
                    index++;
                }
            }
            // 指定された範囲内の要素を部分的にソート。特定の位置にある要素を正確に配置するアルゴリズム。ソートされていない要素全体をソートするよりも効率的に中央値や任意の位置の要素を取得できる。
            std::nth_element(values.begin(), values.begin() + values.size() / 2, values.end());

            // 中央値を新値として設定
            output.at<uchar>(y, x) = values[values.size() / 2 + values.size() / 4];
        }
    }
}
    


int main() {

    auto start_time = std::chrono::high_resolution_clock::now();

    cv::Mat img = cv::imread("30115512.tiff", cv::IMREAD_GRAYSCALE); // グレースケールとして読み込む

    cv::Mat output = img.clone();

    if (img.empty()) {
        std::cerr << "Failed to read img: " << std::endl;
        return -1;
    }

    // メディアンフィルタの適用
    medianFilter(img, output, 3);

    // ウィンドウに表示
    cv::imshow("OriginalImage", img);
    cv::imshow("MedianFilter", output);
    cv::imwrite("median.tiff", output);

    // 処理後の時刻を取得
    auto end_time = std::chrono::high_resolution_clock::now();

    // 経過時間を計算
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 経過時間を表示
    std::cout << "Time: " << duration.count() << " milliseconds" << std::endl;

    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}

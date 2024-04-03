// 2値化されていない画像(Grayscale)をカウントして白い点の総数を出して、重心もだしてそれを画像上に表示する。
// 元画像とthreshをかけた画像と、番号を振った画像が出るようにしている。

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>

#define CHECK printf("Line %d\n", __LINE__)

using namespace std;
int n = 0;
int label_number = 0;

class Volume {
public:
    int fWidth; // Volumeデータの幅
    int fHeight; // Volumeデータの高さ
    int fNFrames; // ボリュームに含まれるフレーム（画像スライス）の数
    std::vector<unsigned char *> fData; // Volumeデータを保存する
    std::vector<int> labeledImage;
    std::vector<std::vector<cv::Point3d>> labeledPoints;

    Volume() { fNFrames = 0; }
    Volume(int nframes, int w, int h); // Volumeの初期化を行うコンストラクタ、各スライスのメモリを確保。

    int ReadImages(std::vector<std::string> imagePaths); // 画像パスのベクトルを受け取り、それらの画像を読み込んでVolumeデータに追加する.
    unsigned char GetValue(int x, int y, int z) { return *(fData[z] + x + y * fWidth); } // 指定された位置のピクセル値を取得して、その値を返す。
    void SetValue(int x, int y, int z, unsigned char v) { *(fData[z] + x + y * fWidth) = v; } // 指定されたピクセル値を取得する。
    int NFrames() { return fNFrames; } // スライスの数を返す。
    int Width() { return fWidth; }     // 幅のサイズを返す。
    int Height() { return fHeight; }   // 高さのサイズを返す。
    void Print() { printf("Volume: w=%d h=%d nframes=%d\n", fWidth, fHeight, fNFrames); } // 各Volumeのサイズを表示するデバッグ用のメソッド。
    void Labeling(int x, int y, int z, int label_number, int index, std::vector<int>& labeledImage, std::vector<std::vector<cv::Point3d>>& labeledPoints);
    void Center(std::vector<std::vector<cv::Point3d>>& labeledPoints, double& y_g, double& x_g, double& z_g, std::vector<int>& labeledImage, int label_number);
    // Volume DrawFogInfo();
    void ShowImage(const std::string& windowName, int slice);
    void DrawFogInfo(cv::Mat& img);
};

// Volumeのクラスのコンタクタの作成
Volume::Volume(int nframes, int w, int h): fNFrames(nframes), fWidth(w), fHeight(h) {

    for(int z = 0; z < fNFrames; z++){
        unsigned char *pRaw = new unsigned char [fWidth*fHeight]; // 各フレームに対して、fWidth x fHeight ピクセルのデータを格納するための配列を動的に確保、この配列のポインタを pRaw に保存する。
        memset(pRaw, 0, fWidth*fHeight*sizeof(unsigned char) );   // 配列を0で初期化、初期状態のすべてのピクセルが0となる。
        fData.push_back(pRaw); // fDataベクトルにpRawを追加する。
    }
}

struct FogInfo {
    int label;
    int pixelCount;
    cv::Point3d centroid;
};

std::vector<FogInfo> fogInfos; // 各Fogの情報を格納する

int Volume::ReadImages(std::vector<std::string> imagePaths){
    // 画像を読み込んで表示
    for (const auto &imagePath : imagePaths) { // 与えられた画像パスのベクトルから画像パスを一つずつ取り出してループする。
        // printf("%s\n", imagePath.c_str());  // imagePath という std::string オブジェクトをCスタイルの文字列に変換して、それを %s フォーマット指定子を使用して標準出力に表示しています。これにより、各画像ファイルのパスが表示されます。
        cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE); // グレースケールとして読み込む

        if (image.empty()) {
            std::cerr << "Failed to read image: " << imagePath << std::endl;
            return -1;
        }

        fHeight = image.rows;   // 高さと幅を読み込んで、Volumeクラスのメンバ変数に設定する。
        fWidth = image.cols;

        unsigned char *pRaw = new unsigned char [fWidth * fHeight]; // ピクセルデータを格納するための配列を自動的に保存する。
        for(int y = 0; y < fHeight; y++){
            for(int x = 0; x < fWidth; x++){
                pRaw[x + y * fWidth] = image.at<uchar>(y, x); // pRawの一次元配列に、各スライスの輝度値を格納する。
            }
        }
        fData.push_back(pRaw);

        // 現在の画像を表示
        // cv::imshow(" Volume Slices", image);

        // 画像間に遅延を追加（例：100ミリ秒）
        cv::waitKey(100);
        fNFrames++;
        // cout << fNFrames << endl;
    }
}

void Volume::Labeling(int x, int y, int z, int label_number, int index, std::vector<int>& labeledImage, std::vector<std::vector<cv::Point3d>>& labeledPoints) {

static int depth=0;
static int finish=0;
depth++;
    n++;
    int tn = n;
    

    labeledImage[index] = label_number;

    if (labeledPoints.size() < label_number) {
        labeledPoints.resize(label_number);
    }

    labeledPoints[label_number - 1].emplace_back(x, y, z); // 座標を格納

    for (int m = -1; m <= 1; m++){
        for (int k = -1; k <= 1; k++) {
            for (int l = -1; l <= 1; l++) {

                int newCol = x + l;
                int newRow = y + k;
                int newframe = z + m;
                int newindex = (newframe * fHeight + newRow) * fWidth + newCol;
                
                if (newRow < 0 || newRow >= fHeight || newCol < 0 || newCol >= fWidth || newframe < 0 || newframe >= fNFrames){
                    continue;
                }

                if(labeledImage[newindex]!=0) continue;

                // printf("LN %d, depth %d, finish %d %d %d %d\n", label_number, depth, finish, newCol, newRow, newframe);
                if (GetValue(newCol, newRow, newframe) > 0) {
                    // cout << n << endl;
                    // n++;
                    Labeling(newCol, newRow, newframe, label_number, newindex, labeledImage, labeledPoints);
                    //LabelingVolume.SetValue(x, y, z, label_number);  // ラベル付けされたピクセルに label_number をセット
                }
                else labeledImage[newindex] = -2;

            }
        }
    }
    // cout << "finish " << tn << "th task" << endl;
    depth--;
    finish++;
}

void Volume::Center(std::vector<std::vector<cv::Point3d>>& labeledPoints, double& y_g, double& x_g, double& z_g, std::vector<int>& labeledImage, int label_number) {

    double sum = 0;
    double x_sum = 0;
    double y_sum = 0;
    double z_sum = 0;

    for (int k = 0; k < labeledPoints[label_number - 1].size(); k++) {

        int x_coord = labeledPoints[label_number - 1][k].x;  // x 座標を取得
        int y_coord = labeledPoints[label_number - 1][k].y;  // y 座標を取得   
        int z_coord = labeledPoints[label_number - 1][k].z;  // z 座標を取得

            sum += GetValue(x_coord, y_coord, z_coord);
            x_sum += x_coord * GetValue(x_coord, y_coord, z_coord);
            y_sum += y_coord * GetValue(x_coord, y_coord, z_coord);
            z_sum += z_coord * GetValue(x_coord, y_coord, z_coord);
        
        }

    if (sum > 0) {

        x_g = x_sum / sum;
        y_g = y_sum / sum;
        z_g = z_sum / sum;

        FogInfo fogInfo;
        fogInfo.label = fogInfos.size() + 1;
        fogInfo.centroid = cv::Point3d(x_g, y_g, z_g);
        fogInfos.push_back(fogInfo);

        std::cout << "Fog_number:" << label_number << std::endl;
        std::cout << "Centroid : (" << x_g << ", " << y_g << ", "<< z_g << ")" << std::endl << std::endl;


    }
}

void Volume::DrawFogInfo(cv::Mat& image) {
    for (const auto& fogInfo : fogInfos) {
        cv::putText(image, std::to_string(fogInfo.label), cv::Point(fogInfo.centroid.x, fogInfo.centroid.y), cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(255), 1, cv::LINE_AA);
    }
}

// 2つのFogInfoの重心座標から距離を計算する関数
double calculateDistance(const FogInfo& fog1, const FogInfo& fog2) {
    double dx = fog2.centroid.x - fog1.centroid.x;
    double dy = fog2.centroid.y - fog1.centroid.y;
    double dz = fog2.centroid.z - fog1.centroid.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

void Volume::ShowImage(const std::string& windowName, int slice) {
    cv::Mat image(fHeight, fWidth, CV_8U);

    for (int y = 0; y < fHeight; y++) {
        for (int x = 0; x < fWidth; x++) {
            image.at<uchar>(y, x) = GetValue(x, y, slice);
        }
    }
    cv::imshow(windowName, image);
    cv::waitKey(100);
}


int main(){

    std::vector<std::string> imagePaths;

    for (int i = 0; i <= 69; i++){
        imagePaths.push_back("C:\\Users\\user\\prg\\uchigo\\3dreco\\F05262\\flat_fielded\\3D_nichikathreshImage" + std::to_string(i) + ".tif");
    }

    Volume vol;                   // 先のVolumeクラスの作成でvolインスタンスを作成し、ReadImagesメソッドを使用して画像を読み込む。
    vol.ReadImages(imagePaths);

    int nz = vol.NFrames();  // volumeデータ内のフレームの枚数
    int h = vol.Height();    // volumeデータの高さ
    int w = vol.Width();     // volumeデータの幅

    std::vector<std::vector<cv::Point3d>> labeledPoints(1000000);
 
    // 画像のピクセル数分の領域を予め確保し、0で初期化
    std::vector<int> labeledImage(h * w * nz, 0);

    // ラベル・重心の初期化(初期値は全て0) 
    int index = 0;      
    double x_g = 0;
    double y_g = 0;
    double z_g = 0;

    for (int z = 0; z < nz; z++){
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                index = (z * h + y) * w + x;  // 各ピクセルの座標

                // printf("LINE %d, i,j = %d, %d\n", __LINE__, i, j);
                
                if (vol.GetValue(x, y, z) == 0){  
                    labeledImage[index] = -1;  // 配列に注意‼　index番目のlabelを-1と定める。
                    continue;  
                }

                if (vol.GetValue(x, y, z) != 0 && labeledImage[index] == 0){  // ピクセルの値が255でかつまだラベリングされていない場合
                    label_number++;
                    vol.Labeling(x, y, z, label_number, index, labeledImage, labeledPoints); // Labeling関数の呼び出し
                    vol.Center(labeledPoints, y_g, x_g, z_g, labeledImage, label_number);
                }
            }
        }
    }

    cv::Mat image(h, w, CV_8U);

    vol.DrawFogInfo(image);

    const std::string fileName = "LabeledPoints3D.txt";

    // ファイルを開く
    std::ofstream outFile(fileName);

    if (!outFile.is_open()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return -1;
    }

    int max_size = 0;
    for (int i = 0; i < label_number; i++){
        // cout << i + 1 << " " << labeledPoints[i] << endl;
        int current_size = labeledPoints[i].size();
        if(current_size > max_size){
            max_size = current_size; // 探索していく中での最大値が分かる。
        }
        cout << i + 1 << " " << labeledPoints[i].size() << endl;
        outFile << labeledPoints[i].size() << endl;
    }

    cout << "maxsize:" << max_size << endl;


/*
    const std::string fileName = "distances3D.txt";

    // ファイルを開く
    std::ofstream outFile(fileName);

    if (!outFile.is_open()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return -1;
    }

    // Fog_number1とFog_number2からFog_number127までの距離を計算し表示
    for (int i = 0; i < fogInfos.size(); ++i) {
        for (int j = i + 1; j < fogInfos.size(); ++j) {
            int distance = calculateDistance(fogInfos[i], fogInfos[j]);
            // std::cout << "Distance between Fog_number" << fogInfos[i].label << " and Fog_number" << fogInfos[j].label << ": " << distance << std::endl;
            cout << distance << endl;
            outFile << distance << endl;
        }
    }
*/

/*
    // Fog_number1とFog_number2からFog_number127までの距離を計算し表示
    for (int j = 1; j < fogInfos.size(); j++) {
        int distance = calculateDistance(fogInfos[0], fogInfos[j]);
        outFile << distance << endl;
    }
*/

    // ファイルを閉じる
    outFile.close();

    // std::cout << "Distances saved to " << fileName << std::endl;

    std::cout << "Total Fog : " << label_number << std::endl;

    // 100µm/320pixelで計算している
    // std::cout << "Density1 : " << label_number/((5.0/16)*w * (5.0/16)*h * 68) << "(\u03BCm)^3" << std::endl;
    // std::cout << "Density2 : " << label_number * 10000 * 256 << "/10000(\u03BCm)^3" << std::endl;
    // std::cout << "Density2 : " << (25.0 * 68.0 * w * h) << "/10000(\u03BCm)^3" << std::endl;
    // double temp_result = label_number * 10000 * 256;
    // double final_result = 25.0 * 68.0 * w * h;
    // std::cout << "Density2 : " << temp_result / final_result << "/10000(\u03BCm)^3" << std::endl;

    // std::cout << std::fixed << std::setprecision(6) << "Density2 : " << label_number * 10000 * 256 / 10000 << "/10000(\u03BCm)^3" << std::endl;

    // std::cout << "Density2 : " << label_number * 10000 * 256/(25.0 * 68.0 * w * h) << "/10000(\u03BCm)^3" << std::endl;


    for (int z = 0; z <= 69; z++){
        vol.ShowImage("fog", z);
        // cv::waitKey(100);
    }

    return 0;  
}





/*
    for (int k = 1; k < labeledPoints.size() + 1; k++){
        std::cout << k << " : " << labeledPoints[k - 1].size() << std::endl;
    }
*/

/*
    for (int i = 0; i < labeledPoints.size(); i++) {
    std::cout << "Label " << i << " Points: ";
        for (int j = 0; j < labeledPoints[i].size(); j++) {
            std::cout << "(" << labeledPoints[i][j].x << ", " << labeledPoints[i][j].y << ") ";
        }
        std::cout << std::endl;
*/

  

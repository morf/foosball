#include <iostream>
#include <filesystem>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

#include "aruco.hpp"
#include "table.hpp"
#include "cameraCalibration.h"
#include "cxxopts.hpp"

cxxopts::ParseResult parseConfiguration(cxxopts::Options &options, int argc, const char *argv[])
{
    try
    {
        const auto config = options.parse(argc, argv);
        if (!config.count("input_path") || config.count("help"))
        {
            std::cerr << options.help({ "" }) << std::endl;
            exit(EXIT_FAILURE);
        }
        return config;
    }
    catch (const cxxopts::OptionException& ex)
    {
        std::cerr << "FAILURE: Error while parsing options (" << ex.what() << ")\n";
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char *argv[])
{
    cxxopts::Options options(argv[0], "Implementacje Przemyslowe");
    options.add_options("")
        ("h,help", "Display help")
        ("i,input_path", "Input video file path", cxxopts::value<std::string>())
        ("d,aruco_path", "Path to aruco dictionary", cxxopts::value<std::string>());

    const auto config = parseConfiguration(options, argc, argv);
    const std::string input_path = config["input_path"].as<std::string>();
    const std::string aruco_path = config["aruco_path"].as<std::string>();

    if (!std::experimental::filesystem::exists(input_path))
    {
        std::cerr << "FAILURE: Input file \"" << input_path << "\" does not exist.\n";
        exit(EXIT_FAILURE);
    }

    cv::Mat frame;
    cv::VideoCapture capture(input_path);
    cv::namedWindow("Aruco Demo");

    cv::Ptr<cv::aruco::Dictionary> aruco_dict = aruco::createDictionary(aruco_path, 5);
    cv::Ptr<cv::aruco::DetectorParameters> detector = aruco::loadParametersFromFile("../../data/config-aruco.yaml");

    //std::cout << (detector->adaptiveThreshConstant = 1) << std::endl;

    std::vector<aruco::ArucoMarker> found, rejected;

    calibration::CameraCalibration camCal("../../data/default.xml", "../../data/out_camera_data.xml");
    table::Table t(1200, 600);

    while (1) {
        capture >> frame;
        if (frame.empty())
            break;

        //frame = camCal.getUndistortedImage(frame);

        //cv::aruco::drawMarker(aruco_dict, 3, 250, frame);

        aruco::detectArucoOnFrame(frame, aruco_dict, found, rejected, detector);
        //aruco::drawMarkersOnFrame(frame, found);

        t.updateTableOnFrame(found);
        //t.drawTableOnFrame(frame);
        frame = t.getTableFromFrame(frame);

        cv::imshow("Aruco Demo", frame);

        if (cv::waitKey(10) >= 0) 
            break;
    }
}


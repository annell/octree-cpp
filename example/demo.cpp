//
// Created by Stefan Annell on 2023-05-30.
//

#include "polyscope/polyscope.h"
#include "../src/OctreeCpp.h"
#include "polyscope/point_cloud.h"

template<>
float DistanceBoxSphere(const Boundary<glm::vec3>& Box, const glm::vec3& SphereCenter, float SphereRadius) {
    const glm::vec3 closestPoint = glm::clamp(SphereCenter, Box.Min, Box.Max);
    if (glm::distance(closestPoint, SphereCenter) > SphereRadius) {
        return glm::distance(closestPoint, SphereCenter) - SphereRadius;
    } else {
        return 0.0f;
    }
}

void app1() {
    OctreeCpp<glm::vec3, int> octree({{-1000, -1000, -1000}, {1000, 1000, 1000}});
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dis(0.0f, 1.0f);
    for (int i = 0; i < 1000; i++) {
        DataWrapper<glm::vec3, int> data = {{dis(gen), dis(gen), dis(gen)}, i};
        octree.Add(data);
    }
    {
        auto pointsWrapper = octree.Query(QueryAll<glm::vec3>{});
        std::vector<glm::vec3> points;
        for (auto pw : pointsWrapper) {
            points.push_back(pw.Vector);
        }
        polyscope::registerPointCloud("points", points);
        auto cloud = polyscope::getPointCloud("points");
        cloud->setPointRenderMode(polyscope::PointRenderMode::Quad);
        cloud->setPointRadius(0.00085);
    }
    {
        glm::vec3 midpoint = {10.0f, 10.0f, 10.0f};
        auto pointsWrapper = octree.Query(QueryRadius<glm::vec3>{midpoint, 50.0f});
        std::vector<glm::vec3> points;
        for (auto pw : pointsWrapper) {
            points.push_back(pw.Vector);
        }
        polyscope::registerPointCloud("q1", points);
        auto cloud = polyscope::getPointCloud("q1");
        cloud->setPointRadius(0.002);
        cloud->setPointColor({1.0, 0.0, 0.0});
        cloud->setPointRenderMode(polyscope::PointRenderMode::Quad);

        std::vector<glm::vec3> points2;
        points2.push_back(midpoint);
        polyscope::registerPointCloud("q1_mid", points2);
        auto midcloud = polyscope::getPointCloud("q1_mid");
        midcloud->setPointRadius(50, false);
        midcloud->setPointColor({1.0, 0.0, 0.0});
        midcloud->setTransparency(0.5);
    }
    {
        glm::vec3 midpoint = {-10.0f, -10.0f, -10.0f};
        auto pointsWrapper = octree.Query(QueryRadius<glm::vec3>{midpoint, 50.0f});
        std::vector<glm::vec3> points;
        for (auto pw : pointsWrapper) {
            points.push_back(pw.Vector);
        }
        polyscope::registerPointCloud("q2", points);
        auto cloud = polyscope::getPointCloud("q2");
        cloud->setPointRadius(0.002);
        cloud->setPointColor({0.0, 1.0, 0.0});
        cloud->setPointRenderMode(polyscope::PointRenderMode::Quad);

        std::vector<glm::vec3> points2;
        points2.push_back(midpoint);
        polyscope::registerPointCloud("q2_mid", points2);
        auto midcloud = polyscope::getPointCloud("q2_mid");
        midcloud->setPointRadius(50, false);
        midcloud->setPointColor({0.0, 1.0, 0.0});
        midcloud->setTransparency(0.5);
    }


    polyscope::show();
}
// Parameters which we will set in the callback UI.
int nPts = 2000;
glm::vec3 midpoint = {10.0f, 10.0f, 10.0f};
float radius = 50.0f;

void mySubroutine() {
    OctreeCpp<glm::vec3, int> octree({{-1000, -1000, -1000}, {1000, 1000, 1000}});
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dis(0.0f, 100.0f);
    for (int i = 0; i < nPts; i++) {
        DataWrapper<glm::vec3, int> data = {{dis(gen), dis(gen), dis(gen)}, i};
        octree.Add(data);
    }
    {
        auto pointsWrapper = octree.Query(QueryAll<glm::vec3>{});
        std::vector<glm::vec3> points;
        for (auto pw : pointsWrapper) {
            points.push_back(pw.Vector);
        }
        polyscope::registerPointCloud("points", points);
        auto cloud = polyscope::getPointCloud("points");
        cloud->setPointRenderMode(polyscope::PointRenderMode::Sphere);
    }
    {
        auto pointsWrapper = octree.Query(QueryRadius<glm::vec3>{midpoint, radius});

        std::vector<glm::vec3> points;
        for (auto pw : pointsWrapper) {
            points.push_back(pw.Vector);
        }
        polyscope::registerPointCloud("q1", points);
        auto cloud = polyscope::getPointCloud("q1");
        cloud->setPointRadius(0.006);
        cloud->setPointColor({1.0, 0.0, 0.0});

        std::vector<glm::vec3> points2;
        points2.push_back(midpoint);
        polyscope::registerPointCloud("q1_mid", points2);
        auto midcloud = polyscope::getPointCloud("q1_mid");
        midcloud->setPointRadius(radius, false);
        midcloud->setPointColor({1.0, 0.0, 0.0});
    }
}

// Your callback functions
void myCallback() {

    // Since options::openImGuiWindowForUserCallback == true by default,
    // we can immediately start using ImGui commands to build a UI

    ImGui::PushItemWidth(100); // Make ui elements 100 pixels wide,
    // instead of full width. Must have
    // matching PopItemWidth() below.

    ImGui::SliderInt("num points", &nPts, 0, 10000);             // set a int variable
    ImGui::SliderFloat3("midpoint", glm::value_ptr(midpoint), -100, 100);  // set a float variable
    ImGui::SliderFloat("radius", &radius, 0, 100);  // set a float variable

    if (ImGui::Button("Run query")) {
        // executes when button is pressed
        mySubroutine();
    }
    ImGui::PopItemWidth();
}

void app2() {
    polyscope::state::userCallback = myCallback;
    polyscope::show();
}

int main([[maybe_unused]] int argc,[[maybe_unused]] const char *argv[]) {
    polyscope::init();
    app2();
    return EXIT_SUCCESS;
}
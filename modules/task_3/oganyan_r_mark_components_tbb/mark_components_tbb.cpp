// Copyright 2021 Oganyan Robert

#include "../../modules/task_3/oganyan_r_mark_components_tbb/mark_components_tbb.h"
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <tbb/task_scheduler_init.h>

static const std::vector<std::pair<int, int>> directions{
        {-1, 0},
        {0,  -1},
        {0,  1},
        {1,  0},
};


void bfs(std::vector<int> *img, std::pair<int, int> start,
         int *number, int width, int height) {
    if ((*img)[start.first * width + start.second] != 1) {
        return;
    }
    std::queue<std::pair<int, int>> q;
    q.push({start});
    (*img)[start.first * width + start.second] = ++(*number);
    while (!q.empty()) {
        auto cur{q.front()};
        q.pop();
        for (auto &neighbor : directions) {
            if (cur.first + neighbor.first >= height || cur.first + neighbor.first < 0
                || cur.second + neighbor.second >= width || cur.second + neighbor.second < 0) {
                continue;
            }
            if ((*img)[(cur.first + neighbor.first) * width + cur.second + neighbor.second] == 1) {
                q.push({(cur.first + neighbor.first), cur.second + neighbor.second});
                (*img)[(cur.first + neighbor.first) * width + cur.second + neighbor.second] = (*number);
            }
        }
    }
    return;
}


std::pair<std::vector<int>, int> MarkComponentsSeq(std::vector<int> *img, int height, int width) {
    if ((*img).size() == 0) {
        throw std::invalid_argument("Size of the image cant be negative");
    }
    if (static_cast<int>((*img).size()) != width * height) {
        throw std::invalid_argument("Size of the image is not correct");
    }
    auto img_new = *img;
    int count_comp{1};
    for (int i{0}; i < height; ++i) {
        for (int j{0}; j < width; ++j) {
            bfs(&img_new, {i, j}, &count_comp, width, height);
        }
    }
    return {img_new, count_comp - 1};
}


std::pair<std::vector<int>, int> MarkComponentsParTbb(std::vector<int> *img, int height, int width, int proc_num) {
    if ((*img).size() == 0) {
        throw std::invalid_argument("Size of the image cant be negative");
    }
    if (static_cast<int>((*img).size()) != width * height) {
        throw std::invalid_argument("Size of the image is not correct");
    }
    auto img_new = *img;
    int count_comp{0};
    Disjoint_Set_Union<int> dsu(height * width);
    dsu.Init();

    tbb::task_scheduler_init init(proc_num);
    tbb::parallel_for(tbb::blocked_range<int>(0, height, height/proc_num + 1),
                      [&img_new, &dsu, width, height](tbb::blocked_range<int> block) {
                          for (auto i = block.begin(); i != block.end(); ++i) {
                              for (int j = 0; j < width; ++j) {
                                  if (img_new[i * width + j]) {
                                      for (auto &neighbor : directions) {
                                          if (i + neighbor.first >= height || i + neighbor.first < 0
                                              || j + neighbor.second >= width || j + neighbor.second < 0) {
                                              continue;
                                          }
                                          int cur = (i + neighbor.first) * width + j + neighbor.second;
                                          if (img_new[cur] == 0) {
                                              continue;
                                          }
                                          dsu.union_sets(i * width + j, cur);
                                      }
                                  }
                              }
                          }
                      });

    init.terminate();
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int cur = i * width + j;
            if (img_new[cur]) {
                img_new[cur] = dsu.find_set(cur, cur) + 1;
            }
            if (img_new[cur] == cur + 1) {
                count_comp++;
            }
        }
    }

    return {img_new, count_comp};
}





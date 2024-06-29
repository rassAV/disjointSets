#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <set>

// �������� JSON ����� � �������������� ���������� ������� � ���� vector<int> dst_list
template <template <typename, typename...> class Vector, typename T, typename... Args>
void load_json(const std::string& file_name, Vector<T, Args...>& dst_list) {
    std::ifstream file(file_name);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_name << std::endl;
        return;
    }

    std::string line;
    std::string json_content;
    while (std::getline(file, line)) {
        json_content += line;
    }

    json_content.erase(std::remove(json_content.begin(), json_content.end(), '['), json_content.end());
    json_content.erase(std::remove(json_content.begin(), json_content.end(), ']'), json_content.end());
    json_content.erase(std::remove(json_content.begin(), json_content.end(), ' '), json_content.end());

    std::stringstream ss(json_content);
    std::string number;
    while (std::getline(ss, number, ',')) {
        dst_list.push_back(std::stoi(number));
    }

    std::cout << "Loaded " << dst_list.size() << " destination IDs from " << file_name << std::endl;
}

// �������� ���� ������ (������ �� �������� ������ � ���������) �� CSV ����� � ���� map<int, vector<int>> src_to_dsts
template <template <typename, typename...> class Map, typename Key, typename Value, typename... Args>
void load_data(const std::string& csv_file_name, Map<Key, Value, Args...>& src_to_dsts) {
    std::ifstream file(csv_file_name);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << csv_file_name << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string src_id_str, dst_id_str;
        std::getline(ss, src_id_str, ';');
        std::getline(ss, dst_id_str, ';');
        int src_id = std::stoi(src_id_str);
        int dst_id = std::stoi(dst_id_str);

        src_to_dsts[src_id].push_back(dst_id);
    }

    int total_connections = 0;
    for (const auto& pair : src_to_dsts) {
        total_connections += pair.second.size();
    }
    std::cout << "Loaded " << total_connections << " connections from " << csv_file_name << std::endl;
}

// ��������, ������ ������������� (� dst_list) ������ (������ �� �������� ������ � ���������) �� CSV ����� � ���� map<int, vector<int>> src_to_dsts
template <
    template <typename, typename...> class Vector, typename T, typename... VecArgs,
    template <typename, typename...> class Map, typename Key, typename Value, typename... Args>
void load_data_filtered(const std::string& csv_file_name, Vector<T, VecArgs...>& dst_list, Map<Key, Value, Args...>& src_to_dsts) {
    std::ifstream file(csv_file_name);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << csv_file_name << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string src_id_str, dst_id_str;
        std::getline(ss, src_id_str, ';');
        std::getline(ss, dst_id_str, ';');
        int src_id = std::stoi(src_id_str);
        int dst_id = std::stoi(dst_id_str);

        if (std::find(dst_list.begin(), dst_list.end(), dst_id) != dst_list.end()) {
            src_to_dsts[src_id].push_back(dst_id);
        }
    }

    int total_connections = 0;
    for (const auto& pair : src_to_dsts) {
        total_connections += pair.second.size();
    }
    std::cout << "Loaded " << total_connections << " connections from (filtered by json) " << csv_file_name << std::endl;
}

// ����������� ���������������� �������� � group_to_srcs � group_to_dsts � ���� map<int, vector<int>>
template <template <typename, typename...> class Map, typename Key, typename Value, typename... Args>
void group_data(const Map<Key, Value, Args...>& src_to_dsts, Map<Key, Value, Args...>& group_to_srcs, Map<Key, Value, Args...>& group_to_dsts) {
    std::map<int, int> dst_to_group;
    int next_group_id = 0;

    // �������� �� ���� SRC_ID � �� DST_IDs
    for (const auto& pair : src_to_dsts) {
        int src_id = pair.first;
        const auto& dst_ids = pair.second;

        std::set<int> groups_to_merge; // ������, ������� ����� ���������� � ������� SRC_ID � ��� DST_IDs

        // ������� ��� ������, ������� ����� ���������� � ������� SRC_ID
        for (int dst_id : dst_ids) {
            const auto& group = dst_to_group.find(dst_id);
            if (group != dst_to_group.end()) {
                groups_to_merge.insert(group->second);
            }
        }
        // ���� ��� ����� ��� �����������, ������� ����� ������
        if (groups_to_merge.empty()) {
            group_to_srcs[next_group_id].push_back(src_id);
            for (int dst_id : dst_ids) {
                dst_to_group[dst_id] = next_group_id;
                group_to_dsts[next_group_id].push_back(dst_id);
            }
            next_group_id++;
        }
        else {
            int new_group_id = *groups_to_merge.begin(); // ����� ���������� ������ ��� �����������
            group_to_srcs[new_group_id].push_back(src_id); // ����������� SRC_ID � DST_IDs ��������� ������
            for (int dst_id : dst_ids) {
                dst_to_group[dst_id] = new_group_id;
                group_to_dsts[new_group_id].push_back(dst_id);
            }
            // ���������� ��������� ������ � ���������
            for (auto it = std::next(groups_to_merge.begin()); it != groups_to_merge.end(); ++it) {
                int group_to_merge = *it;
                for (int src : group_to_srcs[group_to_merge]) {
                    group_to_srcs[new_group_id].push_back(src);
                }
                for (int dst_id : group_to_dsts[group_to_merge]) {
                    dst_to_group[dst_id] = new_group_id;
                    group_to_dsts[new_group_id].push_back(dst_id);
                }
                //������� ������ ������ � �������
                group_to_srcs.erase(group_to_merge);
                group_to_dsts.erase(group_to_merge);
            }
        }
    }

    std::cout << "Number of groups: " << group_to_srcs.size() << std::endl;
    for (const auto& pair : group_to_srcs) {
        std::cout << "Group ID " << pair.first << " have " << pair.second.size() << " source IDs" << std::endl;
    }
}
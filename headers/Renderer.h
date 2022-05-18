#pragma once
#include <SFML/Graphics.hpp>
#include <fstream>
#include <cmath>
#include <random>


class Renderer {
private:
    class Settings {
    public:
        int window_w, window_h, window_step, samples, max_ref, window_step_x, window_step_y;
        bool mouseHidden;
        float speed, offset_step, max_dist;
        sf::Vector3f pos, offset;
        sf::Vector2f lightPos;
        Settings() = default;
        explicit Settings(const std::string &file) {
            std::ifstream in(file);
            in >> window_w >> window_h;
            window_step_x = window_w / 2;
            window_step_y = window_h / 2;
            in >> speed >> offset_step >> window_step;
            mouseHidden = true;
            in >> samples >> max_ref >> max_dist;
            in >> pos.x >> pos.y >> pos.z;
            in >> offset.x >> offset.y >> offset.z;
            in >> lightPos.x >> lightPos.y;
            in.close();
        }
    };

    void load_objects(const std::string &file);
    void process_key(const sf::Keyboard::Key& code, sf::RenderWindow& window, sf::Vector3f& dir);
    void window_setup(sf::RenderWindow& window, sf::RenderTexture& emptyTexture, sf::Shader& shader);

    const Settings base_settings;
    Settings curr_settings;
    std::vector<sf::Glsl::Vec4> sph_cord;
    std::vector<sf::Glsl::Vec4> sph_col;
    std::vector<sf::Glsl::Vec3> box_cord;
    std::vector<sf::Glsl::Vec4> box_col;
    std::vector<sf::Glsl::Mat3> tri_cord;
    std::vector<sf::Glsl::Vec4> tri_col;
    const std::string shader_name;

public:

    Renderer(const std::string &settings_file = "input",
        const std::string &objects_file = "objects",
        std::string shader_file = "Shader.frag") :
        base_settings(settings_file), shader_name(std::move(shader_file)) {
        curr_settings = base_settings;
        load_objects(objects_file);
    }
    void run();
};
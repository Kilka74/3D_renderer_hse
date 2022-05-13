#include <cmath>
#include <random>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <string>
#include <utility>

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

    void load_objects(const std::string &file) {
        std::ifstream in(file);
        int count;
        std::string name;
        in >> count;
        for (int i = 0; i < count; ++i) {
            in >> name;
            if (name == "sph") {
                sf::Glsl::Vec4 cord, col;
                in >> cord.x >> cord.y >> cord.z >> cord.w;
                in >> col.x >> col.y >> col.z >> col.w;
                sph_cord.push_back(cord);
                sph_col.push_back(col);
            } else if (name == "box") {
                sf::Glsl::Vec4 col;
                sf::Glsl::Vec3 cord, size;
                in >> cord.x >> cord.y >> cord.z >> size.x >> size.y >> size.z;
                in >> col.x >> col.y >> col.z >> col.w;
                box_cord.push_back(cord);
                box_cord.push_back(size);
                box_col.push_back(col);
            } else if (name == "tri") {
                sf::Glsl::Vec3 v0, v1, v2;
                sf::Glsl::Vec4 col;
                in >> v0.x >> v0.y >> v0.z >> v1.x >> v1.y >> v1.z >> v2.x >> v2.y >> v2.z;
                in >> col.x >> col.y >> col.z >> col.w;
                tri_cord.push_back(sf::Glsl::Mat3({v0.x, v0.y, v0.z,
                                                   v1.x, v1.y, v1.z,
                                                   v2.x, v2.y, v2.z}));
                tri_col.push_back(col);
            }
        }
        in.close();
    }

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
    explicit Renderer(const std::string &settings_file = "input",
             const std::string &objects_file = "objects",
             std::string shader_file = "Shader.frag") :
            base_settings(settings_file), shader_name(std::move(shader_file)) {
        curr_settings = base_settings;
        load_objects(objects_file);
    }

    void run() {
        sf::RenderWindow window(sf::VideoMode(curr_settings.window_w, curr_settings.window_h),
                                "Ray tracing",
                                sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(40);
        window.setMouseCursorVisible(false);

        sf::RenderTexture emptyTexture;
        emptyTexture.create(curr_settings.window_w, curr_settings.window_h);
        sf::Sprite emptySprite = sf::Sprite(emptyTexture.getTexture());

        sf::Shader shader;
        shader.loadFromFile(shader_name, sf::Shader::Fragment);

        shader.setUniform("u_resolution", sf::Vector2f(float(curr_settings.window_w), float(curr_settings.window_h)));
        shader.setUniform("u_light", curr_settings.lightPos);
        shader.setUniform("u_samples", curr_settings.samples > 0 ? curr_settings.samples : 1);
        shader.setUniform("u_refs", curr_settings.max_ref);
        shader.setUniform("u_max_dist", curr_settings.max_dist);

        std::random_device RandomGenerator;
        std::mt19937 e2(RandomGenerator());
        std::uniform_real_distribution<> dist(0.0f, 1.0f);

        shader.setUniform("u_sph_num", int(sph_col.size()));
        shader.setUniformArray("u_sph_cord", sph_cord.data(), sph_cord.size());
        shader.setUniformArray("u_sph_col", sph_col.data(), sph_col.size());

        shader.setUniform("u_box_num", int(box_col.size()));
        shader.setUniformArray("u_box_cord", box_cord.data(), box_cord.size());
        shader.setUniformArray("u_box_col", box_col.data(), box_col.size());

        shader.setUniform("u_tri_num", int(tri_col.size()));
        shader.setUniformArray("u_tri_cord", tri_cord.data(), tri_cord.size());
        shader.setUniformArray("u_tri_col", tri_col.data(), tri_col.size());

        bool change = true;
        while (window.isOpen()) {
            sf::Event event{};
            sf::Vector3f dir = sf::Vector3f(0.0f, 0.0f, 0.0f);
            while (window.pollEvent(event)) {
                change = true;
                if (event.type == sf::Event::Closed) {
                    window.close();
                } else if (event.type == sf::Event::MouseButtonPressed) {
                    window.setMouseCursorVisible(false);
                    curr_settings.mouseHidden = true;
                } else if (event.type == sf::Event::KeyPressed) {
                    switch (event.key.code) {
                        case sf::Keyboard::Escape:
                            window.setMouseCursorVisible(true);
                            curr_settings.mouseHidden = false;
                            break;
                        case sf::Keyboard::W:
                            dir += {1.0, 0.0, 0.0};
                            break;
                        case sf::Keyboard::A:
                            dir += {0.0, -1.0, 0.0};
                            break;
                        case sf::Keyboard::S:
                            dir += {-1.0, 0.0, 0.0};
                            break;
                        case sf::Keyboard::D:
                            dir += {0.0, 1.0, 0.0};
                            break;
                        case sf::Keyboard::Space:
                            curr_settings.pos.z -= curr_settings.speed;
                            break;
                        case sf::Keyboard::C:
                            curr_settings.pos.z += curr_settings.speed;
                            break;
                        case sf::Keyboard::Left:
                            curr_settings.window_step_x -= curr_settings.window_step;
                            break;
                        case sf::Keyboard::Right:
                            curr_settings.window_step_x += curr_settings.window_step;
                            break;
                        case sf::Keyboard::Up:
                            curr_settings.window_step_y -= curr_settings.window_step;
                            break;
                        case sf::Keyboard::Down:
                            curr_settings.window_step_y += curr_settings.window_step;
                            break;
                        case sf::Keyboard::Enter:
                            curr_settings = base_settings;
                            break;
                        case sf::Keyboard::Period:
                            curr_settings.offset.x += curr_settings.offset_step;
                            break;
                        case sf::Keyboard::Comma:
                            curr_settings.offset.x -= curr_settings.offset_step;
                            break;
                        case sf::Keyboard::Quote:
                            curr_settings.offset.y += curr_settings.offset_step;
                            break;
                        case sf::Keyboard::Semicolon:
                            curr_settings.offset.y -= curr_settings.offset_step;
                            break;
                        case sf::Keyboard::LBracket:
                            curr_settings.offset.z += curr_settings.offset_step;
                            break;
                        case sf::Keyboard::RBracket:
                            curr_settings.offset.z -= curr_settings.offset_step;
                            break;
                        default:
                            break;
                    }
                }
            }
            if (curr_settings.mouseHidden && change) {
                float mx = (float(curr_settings.window_step_x) / float(curr_settings.window_w) - 0.5f);
                float my = (float(curr_settings.window_step_y) / float(curr_settings.window_h) - 0.5f);
                sf::Vector3f dirTemp;
                dirTemp.z = dir.z * std::cos(-my) - dir.x * std::sin(-my);
                dirTemp.x = dir.z * std::sin(-my) + dir.x * std::cos(-my);
                dirTemp.y = dir.y;
                dir.x = dirTemp.x * std::cos(mx) - dirTemp.y * std::sin(mx);
                dir.y = dirTemp.x * std::sin(mx) + dirTemp.y * std::cos(mx);
                dir.z = dirTemp.z;
                curr_settings.pos += dir * curr_settings.speed;

                shader.setUniform("u_offset", curr_settings.offset);
                shader.setUniform("u_pos", curr_settings.pos);
                shader.setUniform("u_angle", sf::Vector2f(mx, my));
                shader.setUniform("u_seed1", sf::Vector2f((float) dist(e2), (float) dist(e2)) * 999.0f);
                shader.setUniform("u_seed2", sf::Vector2f((float) dist(e2), (float) dist(e2)) * 999.0f);
                window.draw(emptySprite, &shader);
                window.display();
            }
            change = false;
        }
    }
};

int main() {
    Renderer renderer;
    renderer.run();
    return 0;
}
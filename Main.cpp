#include <cmath>
#include <random>
#include <SFML/Graphics.hpp>
#include <fstream>

int w, h, stepX, stepY, step_angle;
float speed, step_offset;
bool mouseHidden;
int samples, refs;
float max_dist;
sf::Vector3f pos;
sf::Vector3f offset;
sf::Vector2f lightPos;
std::vector<sf::Glsl::Vec4> sph_cord;
std::vector<sf::Glsl::Vec4> sph_col;
std::vector<sf::Glsl::Vec3> box_cord;
std::vector<sf::Glsl::Vec4> box_col;
std::vector<sf::Glsl::Mat3> tri_cord;
std::vector<sf::Glsl::Vec4> tri_col;

void base() {
    std::ifstream in("input");
    in >> w >> h;
    stepX = w / 2;
    stepY = h / 2;
    in >> speed >> step_offset >> step_angle;
    mouseHidden = true;
    in >> samples >> refs >> max_dist;
    in >> pos.x >> pos.y >> pos.z;
    in >> offset.x >> offset.y >> offset.z;
    in >> lightPos.x >> lightPos.y;
    in.close();
}

void load_objects() {
    std::ifstream in("objects");
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

int main() {
    base();
    load_objects();
    sf::RenderWindow window(sf::VideoMode(w, h), "Ray tracing", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(40);
    window.setMouseCursorVisible(false);

    sf::RenderTexture emptyTexture;
    emptyTexture.create(w, h);
    sf::Sprite emptySprite = sf::Sprite(emptyTexture.getTexture());

    sf::Shader shader;
    shader.loadFromFile("Shader.frag", sf::Shader::Fragment);

    shader.setUniform("u_resolution", sf::Vector2f(float(w), float(h)));
    shader.setUniform("u_light", lightPos);
    shader.setUniform("u_samples", samples > 0 ? samples : 1);
    shader.setUniform("u_refs", refs);
    shader.setUniform("u_max_dist", max_dist);

    std::random_device rd;
    std::mt19937 e2(rd());
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


    while (window.isOpen()) {
        bool change = false;
        sf::Event event{};
        sf::Vector3f dir = sf::Vector3f(0.0f, 0.0f, 0.0f);
        while (window.pollEvent(event)) {
            change = true;
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                window.setMouseCursorVisible(false);
                mouseHidden = true;
            } else if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Escape:
                        window.setMouseCursorVisible(true);
                        mouseHidden = false;
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
                        pos.z -= speed;
                        break;
                    case sf::Keyboard::C:
                        pos.z += speed;
                        break;
                    case sf::Keyboard::Left:
                        stepX -= step_angle;
                        break;
                    case sf::Keyboard::Right:
                        stepX += step_angle;
                        break;
                    case sf::Keyboard::Up:
                        stepY -= step_angle;
                        break;
                    case sf::Keyboard::Down:
                        stepY += step_angle;
                        break;
                    case sf::Keyboard::Enter:
                        base();
                        break;
                    case sf::Keyboard::Period:
                        offset.x += step_offset;
                        break;
                    case sf::Keyboard::Comma:
                        offset.x -= step_offset;
                        break;
                    case sf::Keyboard::Quote:
                        offset.y += step_offset;
                        break;
                    case sf::Keyboard::Semicolon:
                        offset.y -= step_offset;
                        break;
                    case sf::Keyboard::LBracket:
                        offset.z -= step_offset;
                        break;
                    case sf::Keyboard::RBracket:
                        offset.z += step_offset;
                        break;
                    default:
                        break;
                }
            }
        }
        if (mouseHidden && change) {
            float mx = (float(stepX) / float(w) - 0.5f);
            float my = (float(stepY) / float(h) - 0.5f);
            sf::Vector3f dirTemp;
            dirTemp.z = dir.z * std::cos(-my) - dir.x * std::sin(-my);
            dirTemp.x = dir.z * std::sin(-my) + dir.x * std::cos(-my);
            dirTemp.y = dir.y;
            dir.x = dirTemp.x * std::cos(mx) - dirTemp.y * std::sin(mx);
            dir.y = dirTemp.x * std::sin(mx) + dirTemp.y * std::cos(mx);
            dir.z = dirTemp.z;
            pos += dir * speed;

            shader.setUniform("u_offset", offset);
            shader.setUniform("u_pos", pos);
            shader.setUniform("u_angle", sf::Vector2f(mx, my));
            shader.setUniform("u_seed1", sf::Vector2f((float) dist(e2), (float) dist(e2)) * 999.0f);
            shader.setUniform("u_seed2", sf::Vector2f((float) dist(e2), (float) dist(e2)) * 999.0f);
            window.draw(emptySprite, &shader);
            window.display();
        }
    }
    return 0;
}
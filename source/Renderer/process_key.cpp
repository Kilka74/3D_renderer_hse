#include "../../headers/Renderer.h"


void Renderer::process_key(const sf::Keyboard::Key& code, sf::RenderWindow& window, sf::Vector3f& dir) {
    switch (code) {
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

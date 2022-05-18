#include "../../headers/Renderer.h"


void Renderer::window_setup(sf::RenderWindow& window, sf::RenderTexture& emptyTexture, sf::Shader& shader) {
    window.setFramerateLimit(40);
    window.setMouseCursorVisible(false);
    emptyTexture.create(curr_settings.window_w, curr_settings.window_h);
    shader.loadFromFile(shader_name, sf::Shader::Fragment);
    shader.setUniform("u_resolution", sf::Vector2f(float(curr_settings.window_w), float(curr_settings.window_h)));
    shader.setUniform("u_light", curr_settings.lightPos);
    shader.setUniform("u_samples", curr_settings.samples > 0 ? curr_settings.samples : 1);
    shader.setUniform("u_refs", curr_settings.max_ref);
    shader.setUniform("u_max_dist", curr_settings.max_dist);
    shader.setUniform("u_sph_num", int(sph_col.size()));
    shader.setUniformArray("u_sph_cord", sph_cord.data(), sph_cord.size());
    shader.setUniformArray("u_sph_col", sph_col.data(), sph_col.size());
    shader.setUniform("u_box_num", int(box_col.size()));
    shader.setUniformArray("u_box_cord", box_cord.data(), box_cord.size());
    shader.setUniformArray("u_box_col", box_col.data(), box_col.size());
    shader.setUniform("u_tri_num", int(tri_col.size()));
    shader.setUniformArray("u_tri_cord", tri_cord.data(), tri_cord.size());
    shader.setUniformArray("u_tri_col", tri_col.data(), tri_col.size());
}

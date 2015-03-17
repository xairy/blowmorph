// Copyright (c) 2015 Blowmorph Team

#include "client/render_window.h"

#include <map>
#include <string>

#include <SFML/Graphics.hpp>

#include "base/macros.h"
#include "base/pstdint.h"
#include "base/utils.h"

#include "engine/world.h"
#include "engine/utils.h"

#include "client/entity.h"
#include "client/resource_manager.h"
#include "client/sprite.h"
#include "client/utils.h"

namespace bm {

RenderWindow::RenderWindow() : state_(STATE_FINALIZED) { }

RenderWindow::~RenderWindow() {
  if (state_ == STATE_FINALIZED) {
    return;
  }
  if (font_ != NULL) delete font_;
  if (render_window_ != NULL) delete render_window_;
}

void RenderWindow::Initialize() {
  CHECK(state_ == STATE_FINALIZED);

  const Config::ClientConfig& config =
    Config::GetInstance()->GetClientConfig();

  sf::VideoMode video_mode(config.screen_width, config.screen_height);
  sf::Uint32 style = config.fullscreen ?
    sf::Style::Fullscreen : sf::Style::Default;
  render_window_ = new sf::RenderWindow(video_mode, "Blowmorph", style);
  CHECK(render_window_ != NULL);
  view_.reset(sf::FloatRect(0.0f, 0.0f,
      static_cast<float>(config.screen_width),
      static_cast<float>(config.screen_height)));
  render_window_->setView(view_);

  // By default if a key is held, multiple 'KeyPressed' events
  // will be generated. We disable such behaviour.
  render_window_->setKeyRepeatEnabled(false);

  font_ = new sf::Font();
  CHECK(font_ != NULL);
  font_->loadFromFile("data/fonts/tahoma.ttf");

  state_ = STATE_INITIALIZED;
}

void RenderWindow::StartFrame() {
  CHECK(state_ == STATE_INITIALIZED);
  render_window_->clear();
}

void RenderWindow::EndFrame() {
  CHECK(state_ == STATE_INITIALIZED);
  render_window_->display();
}

void RenderWindow::SetViewCenter(sf::Vector2f location) {
  CHECK(state_ == STATE_INITIALIZED);
  view_.setCenter(Round(sf::Vector2f(location.x, location.y)));
  render_window_->setView(view_);
}

void RenderWindow::RenderSprite(Sprite* sprite) {
  CHECK(state_ == STATE_INITIALIZED);
  sprite->Render(render_window_);
}

void RenderWindow::RenderSprites(const std::vector<Sprite*>& sprites) {
  CHECK(state_ == STATE_INITIALIZED);
  for (auto sprite : sprites) {
    RenderSprite(sprite);
  }
}

void RenderWindow::RenderEntity(ClientEntity* entity) {
  CHECK(state_ == STATE_INITIALIZED);

  b2Vec2 b2p = entity->GetPosition();
  sf::Vector2f position = Round(sf::Vector2f(b2p.x, b2p.y));

  Sprite* sprite = entity->GetSprite();
  sprite->SetPosition(position);
  sprite->SetRotation(entity->GetRotation() / M_PI * 180.0f);
  RenderSprite(sprite);

  if (entity->HasCaption()) {
    sf::Vector2f caption_offset = sf::Vector2f(0.0f, -25.0f);
    sf::Vector2f caption_pos = position + caption_offset;
    entity->GetCaption()->setPosition(caption_pos.x, caption_pos.y);
    render_window_->draw(*entity->GetCaption());
  }
}

void RenderWindow::RenderWorld(World* world) {
  CHECK(state_ == STATE_INITIALIZED);
  for (auto i : *world->GetStaticEntities()) {
      ClientEntity* entity = static_cast<ClientEntity*>(i.second);
      RenderEntity(entity);
    }
    for (auto i : *world->GetDynamicEntities()) {
      ClientEntity* entity = static_cast<ClientEntity*>(i.second);
      RenderEntity(entity);
    }
}

void RenderWindow::RenderPlayerStats(int health, int max_health,
                                 int energy, int max_energy) {
  CHECK(state_ == STATE_INITIALIZED);
  sf::Vector2f size = view_.getSize();

  // Initialize transforms for drawing in different corners of the screen.

  size.y = -size.y;
  sf::Transform bottom_left_transform;
  bottom_left_transform.translate(view_.getCenter() - size / 2.0f);

  size.x = -size.x;
  sf::Transform bottom_right_transform;
  bottom_right_transform.translate(view_.getCenter() - size / 2.0f);

  size.x = 0.0f;
  sf::Transform bottom_center_transform;
  bottom_center_transform.translate(view_.getCenter() - size / 2.0f);

  // Draw health and energy bars.

  float health_rect_width = 300.0f * health / max_health;
  float health_rect_height = 15.0f;
  sf::Vector2f health_rect_size(health_rect_width, health_rect_height);
  sf::RectangleShape health_rect;
  health_rect.setSize(health_rect_size);
  health_rect.setPosition(sf::Vector2f(-320.0f, -60.0f));
  health_rect.setFillColor(sf::Color(0xFF, 0x00, 0xFF, 0xBB));
  render_window_->draw(health_rect, bottom_right_transform);

  float energy_rect_width = 300.0f * energy / max_energy;
  float energy_rect_height = 15.0f;
  sf::Vector2f energy_rect_size(energy_rect_width, energy_rect_height);
  sf::RectangleShape energy_charge_rect;
  energy_charge_rect.setSize(energy_rect_size);
  energy_charge_rect.setPosition(sf::Vector2f(-320.0f, -30.0f));
  energy_charge_rect.setFillColor(sf::Color(0x00, 0xFF, 0xFF, 0xBB));
  render_window_->draw(energy_charge_rect, bottom_right_transform);

  // Draw gun slots.

  sf::Vector2f gun_slot_rect_size(50.0f, 50.0f);
  sf::RectangleShape gun_slot_rect;
  gun_slot_rect.setSize(gun_slot_rect_size);
  gun_slot_rect.setPosition(sf::Vector2f(-60.0f, -70.0f));
  gun_slot_rect.setFillColor(sf::Color(0xFF, 0xFF, 0xFF, 0x00));
  gun_slot_rect.setOutlineColor(sf::Color(0xFF, 0x00, 0x00, 0xBB));
  gun_slot_rect.setOutlineThickness(3.0f);
  render_window_->draw(gun_slot_rect, bottom_center_transform);

  gun_slot_rect.setPosition(sf::Vector2f(10.0f, -70.0f));
  gun_slot_rect.setFillColor(sf::Color(0xFF, 0xFF, 0xFF, 0x00));
  gun_slot_rect.setOutlineColor(sf::Color(0x00, 0xFF, 0x00, 0xBB));
  render_window_->draw(gun_slot_rect, bottom_center_transform);
}

void RenderWindow::RenderMinimap(World* world, ClientEntity* player) {
  CHECK(state_ == STATE_INITIALIZED);
  sf::Vector2f size = view_.getSize();

  // Initialize transforms for drawing in different corners of the screen.

  size.x = -size.x;
  sf::Transform top_right_transform;
  top_right_transform.translate(view_.getCenter() - size / 2.0f);

  // Draw minimap.

  float compass_range = 400.0f;
  float compass_radius = 60.0f;
  sf::Vector2f compass_center(-80.0f, 80.0f);

  sf::CircleShape compass_border(compass_radius);
  compass_border.setPosition(compass_center.x - compass_radius,
      compass_center.y - compass_radius);  // Left top corner, not center.
  compass_border.setOutlineColor(sf::Color(0xFF, 0xFF, 0xFF, 0xFF));
  compass_border.setOutlineThickness(1.0f);
  compass_border.setFillColor(sf::Color(0xFF, 0xFF, 0xFF, 0x00));
  render_window_->draw(compass_border, top_right_transform);

  for (auto i : *world->GetDynamicEntities()) {
    Entity* obj = i.second;

    b2Vec2 obj_pos = obj->GetPosition();
    b2Vec2 player_pos = player->GetPosition();
    b2Vec2 rel = obj_pos - player_pos;
    if (Length(rel) < compass_range) {
      rel = (compass_radius / compass_range) * rel;
      sf::CircleShape circle(1.0f);
      circle.setPosition(compass_center + sf::Vector2f(rel.x, rel.y));
      circle.setFillColor(sf::Color(0xFF, 0x00, 0x00, 0xFF));
      render_window_->draw(circle, top_right_transform);
    }
  }

  for (auto i : *world->GetStaticEntities()) {
    Entity* obj = i.second;

    b2Vec2 obj_pos = obj->GetPosition();
    b2Vec2 player_pos = player->GetPosition();
    b2Vec2 rel = obj_pos - player_pos;
    if (Length(rel) < compass_range) {
      rel = (compass_radius / compass_range) * rel;
      sf::CircleShape circle(1.0f);
      circle.setPosition(compass_center + sf::Vector2f(rel.x, rel.y));
      circle.setFillColor(sf::Color(0x00, 0xFF, 0x00, 0xFF));
      render_window_->draw(circle, top_right_transform);
    }
  }

  sf::CircleShape circle(1.0f);
  circle.setPosition(compass_center);
  circle.setFillColor(sf::Color(0x00, 0x00, 0xFF, 0xFF));
  render_window_->draw(circle, top_right_transform);
}

// TODO(xairy): sort players by score.
void RenderWindow::RenderScoretable(
    std::map<uint32_t, int> player_scores_,
    std::map<uint32_t, std::string> player_names_) {
  CHECK(state_ == STATE_INITIALIZED);
  sf::Vector2f size = view_.getSize();

  // Initialize transforms for drawing in different corners of the screen.

  sf::Transform top_left_transform;
  top_left_transform.translate(view_.getCenter() - size / 2.0f);

  size.x = -size.x;
  sf::Transform top_right_transform;
  top_right_transform.translate(view_.getCenter() - size / 2.0f);

  size.y = -size.y;
  sf::Transform bottom_right_transform;
  bottom_right_transform.translate(view_.getCenter() - size / 2.0f);

  size.x = -size.x;
  sf::Transform bottom_left_transform;
  bottom_left_transform.translate(view_.getCenter() - size / 2.0f);

  // Draw scoretable.

  sf::Vector2f view_size = view_.getSize();
  size_t player_count = player_scores_.size();
  sf::Vector2f score_rect_size(400.0f, player_count * 50.0f);
  sf::Vector2f score_rect_position((view_size.x - score_rect_size.x) / 2,
        (view_size.y - score_rect_size.y) / 2);
  sf::RectangleShape score_rect;
  score_rect.setSize(score_rect_size);
  score_rect.setPosition(score_rect_position);
  score_rect.setFillColor(sf::Color(0xAA, 0xAA, 0xAA, 0xBB));
  render_window_->draw(score_rect, top_left_transform);

  int i = 0;
  std::map<uint32_t, int>::iterator it;
  for (it = player_scores_.begin(); it != player_scores_.end(); ++it) {
    std::string score = IntToStr(it->second);
    RenderText(player_names_[it->first], score_rect_position +
        sf::Vector2f(20.0f, 50.0f * i), 40, sf::Color::Blue);
    RenderText(score, score_rect_position +
        sf::Vector2f(score_rect_size.x - 50.0f, 50.0f * i),
        40, sf::Color::Magenta);
    i++;
  }
}

void RenderWindow::RenderText(
    const std::string& str,
    const sf::Vector2f& position,
    int size, sf::Color color) {
  CHECK(state_ == STATE_INITIALIZED);
  sf::Text text;
  text.setString(str);
  text.setCharacterSize(size);
  text.setColor(color);
  text.setStyle(sf::Text::Regular);
  text.setFont(*font_);
  sf::Transform transform;
  transform.translate(view_.getCenter() - view_.getSize() / 2.0f);
  transform.translate(position);
  render_window_->draw(text, transform);
}

sf::Vector2u RenderWindow::GetWindowSize() const {
  CHECK(state_ == STATE_INITIALIZED);
  return render_window_->getSize();
}

sf::Font* RenderWindow::GetFont() {
  CHECK(state_ == STATE_INITIALIZED);
  return font_;
}

sf::Vector2i RenderWindow::GetMousePosition() const {
  CHECK(state_ == STATE_INITIALIZED);
  return sf::Mouse::getPosition(*render_window_);
}

bool RenderWindow::PollEvent(sf::Event* event) {
  CHECK(state_ == STATE_INITIALIZED);
  return render_window_->pollEvent(*event);
}

}  // namespace bm

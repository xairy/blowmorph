entity.h:
virtual void Damage(int damage, uint32_t owner_id) = 0;
 
bool Entity::Collide(Player* player, Bullet* bullet) {
...
bullet->Explode(bullet->_owner_id);
...
}
 
void Bullet::Explode(uint32_t owner_id) {
...
bool rv = _world_manager->Blow(_shape->GetPosition(), owner_id);
...
}
 
bool WorldManager::Blow(const Vector2f& location, uint32_t owner_id) {
...
entity->Damage(damage, owner_id);
...
}
 
Player::Damage(int damage, uint32_t owner_id) {
...
Player* bullet_owner = static_cast<Player*>(_world_manager->GetEntity(owner_id));
bullet_owner->IncScore();
...
}

cd build;make config=release64;cd ..

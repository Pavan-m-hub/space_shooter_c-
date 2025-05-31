#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <memory>
#include <cmath>
#include <random>
#include <algorithm>
#include <map>

// Game states
enum class GameState {
    MainMenu,
    Playing,
    BossFight,
    GameOver,
    Victory
};

// Weapon types
enum class WeaponType {
    Basic,
    Double,
    Triple,
    Laser
};

// Enemy types
enum class EnemyType {
    Basic,
    Fast,
    Tanky,
    Boss
};

// Power-up types
enum class PowerUpType {
    Health,
    Shield,
    WeaponUpgrade,
    ScoreBoost
};

// Particle effect for explosions, etc.
class Particle {
public:
    Particle(const sf::Vector2f& position, const sf::Color& color, float speed, float angle, float lifetime) 
        : position(position), velocity(std::cos(angle) * speed, std::sin(angle) * speed), 
          color(color), lifetime(lifetime), maxLifetime(lifetime) {
        shape.setRadius(3.0f);
        shape.setFillColor(color);
        shape.setPosition(position);
        shape.setOrigin(3.0f, 3.0f);
    }

    bool update(float deltaTime) {
        lifetime -= deltaTime;
        if (lifetime <= 0) return false;

        position += velocity * deltaTime;
        shape.setPosition(position);
        
        // Fade out as lifetime decreases
        float alpha = (lifetime / maxLifetime) * 255;
        sf::Color newColor = color;
        newColor.a = static_cast<sf::Uint8>(alpha);
        shape.setFillColor(newColor);
        
        return true;
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(shape);
    }

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float lifetime;
    float maxLifetime;
    sf::CircleShape shape;
};

// Animation class for sprite animations
class Animation {
public:
    Animation(sf::Texture& texture, int frameCount, float frameTime) 
        : texture(texture), frameCount(frameCount), frameTime(frameTime), currentFrame(0), elapsedTime(0) {
        frameSize = sf::Vector2i(texture.getSize().x / frameCount, texture.getSize().y);
        sprite.setTexture(texture);
        sprite.setTextureRect(sf::IntRect(0, 0, frameSize.x, frameSize.y));
        sprite.setOrigin(frameSize.x / 2.0f, frameSize.y / 2.0f);
    }

    void update(float deltaTime) {
        elapsedTime += deltaTime;
        if (elapsedTime >= frameTime) {
            elapsedTime = 0;
            currentFrame = (currentFrame + 1) % frameCount;
            sprite.setTextureRect(sf::IntRect(currentFrame * frameSize.x, 0, frameSize.x, frameSize.y));
        }
    }

    void setPosition(const sf::Vector2f& position) {
        sprite.setPosition(position);
    }

    void setScale(float scaleX, float scaleY) {
        sprite.setScale(scaleX, scaleY);
    }

    void setRotation(float angle) {
        sprite.setRotation(angle);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }

    bool isFinished() const {
        return currentFrame == frameCount - 1 && elapsedTime >= frameTime;
    }

    void reset() {
        currentFrame = 0;
        elapsedTime = 0;
        sprite.setTextureRect(sf::IntRect(0, 0, frameSize.x, frameSize.y));
    }

private:
    sf::Texture& texture;
    sf::Sprite sprite;
    int frameCount;
    float frameTime;
    int currentFrame;
    float elapsedTime;
    sf::Vector2i frameSize;
};

// Explosion effect
class Explosion {
public:
    Explosion(const sf::Vector2f& position, float scale = 1.0f) : position(position), isActive(true) {
        // Create particles
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angleDist(0, 2 * 3.14159f);
        std::uniform_real_distribution<float> speedDist(50.0f, 200.0f);
        std::uniform_real_distribution<float> lifetimeDist(0.5f, 1.5f);
        
        for (int i = 0; i < 30; ++i) {
            float angle = angleDist(gen);
            float speed = speedDist(gen);
            float lifetime = lifetimeDist(gen);
            
            sf::Color color;
            if (i % 3 == 0) color = sf::Color(255, 60, 0);  // Orange
            else if (i % 3 == 1) color = sf::Color(255, 200, 0);  // Yellow
            else color = sf::Color(255, 0, 0);  // Red
            
            particles.emplace_back(position, color, speed, angle, lifetime);
        }
    }

    bool update(float deltaTime) {
        for (auto it = particles.begin(); it != particles.end();) {
            if (!it->update(deltaTime)) {
                it = particles.erase(it);
            } else {
                ++it;
            }
        }
        
        return !particles.empty();
    }

    void draw(sf::RenderWindow& window) const {
        for (const auto& particle : particles) {
            particle.draw(window);
        }
    }

private:
    sf::Vector2f position;
    std::vector<Particle> particles;
    bool isActive;
};

// Entity class for game objects
class Entity {
public:
    Entity(const std::string& texturePath) {
        if (!texture.loadFromFile(texturePath)) {
            // Handle error
        }
        sprite.setTexture(texture);
        // Center the origin
        sprite.setOrigin(texture.getSize().x / 2.0f, texture.getSize().y / 2.0f);
    }

    virtual void update(float deltaTime) {}

    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }

    void setPosition(const sf::Vector2f& position) {
        sprite.setPosition(position);
    }

    void move(float x, float y) {
        sprite.move(x, y);
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }

    void setScale(float scaleX, float scaleY) {
        sprite.setScale(scaleX, scaleY);
    }

    void setRotation(float angle) {
        sprite.setRotation(angle);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }

protected:
    sf::Texture texture;
    sf::Sprite sprite;
};

// Bullet class
class Bullet : public Entity {
public:
    Bullet(const std::string& texturePath, float damage = 10.0f) 
        : Entity(texturePath), damage(damage) {
        setScale(0.5f, 0.5f);
    }

    void update(float deltaTime) override {
        move(0.f, -speed * deltaTime);
    }

    bool isOffScreen() const {
        return getPosition().y < 0;
    }

    float getDamage() const { return damage; }

private:
    float speed = 600.0f;
    float damage;
};

// Laser class - special weapon
class Laser : public Entity {
public:
    Laser() : Entity("assets/images/weapons/laser.png"), lifetime(0.5f), damage(1.0f) {
        setScale(0.5f, 10.0f);
    }

    void update(float deltaTime) override {
        lifetime -= deltaTime;
    }

    bool isActive() const {
        return lifetime > 0;
    }

    float getDamage() const { return damage; }

private:
    float lifetime;
    float damage;
};

// Shield class
class Shield : public Entity {
public:
    Shield() : Entity("assets/images/effects/shield.png"), health(100.0f), active(false) {
        setScale(1.2f, 1.2f);
    }

    void update(float deltaTime) override {
        if (active) {
            health -= 10.0f * deltaTime; // Shield depletes over time
            if (health <= 0) {
                active = false;
            }
        }
    }

    void activate() {
        health = 100.0f;
        active = true;
    }

    bool isActive() const { return active; }
    float getHealth() const { return health; }

    void takeDamage(float amount) {
        health -= amount;
        if (health <= 0) {
            active = false;
        }
    }

private:
    float health;
    bool active;
};

// Player class
class Player : public Entity {
public:
    Player() : Entity("assets/images/player.png"), health(100), score(0), 
               weaponType(WeaponType::Basic), shieldActive(false) {
        setScale(0.5f, 0.5f);
        shield = std::make_unique<Shield>();
    }

    void update(float deltaTime) override {
        // Player movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && getPosition().x > 0) {
            move(-speed * deltaTime, 0.f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && getPosition().x < 800) {
            move(speed * deltaTime, 0.f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && getPosition().y > 0) {
            move(0.f, -speed * deltaTime);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && getPosition().y < 600) {
            move(0.f, speed * deltaTime);
        }

        // Update shield
        if (shield->isActive()) {
            shield->setPosition(getPosition());
            shield->update(deltaTime);
        }
    }

    bool canShoot() {
        float cooldown = 0.0f;
        switch (weaponType) {
            case WeaponType::Basic: cooldown = 0.25f; break;
            case WeaponType::Double: cooldown = 0.2f; break;
            case WeaponType::Triple: cooldown = 0.15f; break;
            case WeaponType::Laser: cooldown = 1.0f; break;
        }

        if (shootClock.getElapsedTime().asSeconds() > cooldown) {
            shootClock.restart();
            return true;
        }
        return false;
    }

    std::vector<std::shared_ptr<Bullet>> shoot() {
        std::vector<std::shared_ptr<Bullet>> newBullets;
        
        switch (weaponType) {
            case WeaponType::Basic: {
                auto bullet = std::make_shared<Bullet>("assets/images/bullet.png");
                bullet->setPosition(getPosition().x, getPosition().y - 30.f);
                newBullets.push_back(bullet);
                break;
            }
            case WeaponType::Double: {
                auto bullet1 = std::make_shared<Bullet>("assets/images/weapons/bullet1.png", 15.0f);
                auto bullet2 = std::make_shared<Bullet>("assets/images/weapons/bullet1.png", 15.0f);
                bullet1->setPosition(getPosition().x - 20.f, getPosition().y - 20.f);
                bullet2->setPosition(getPosition().x + 20.f, getPosition().y - 20.f);
                newBullets.push_back(bullet1);
                newBullets.push_back(bullet2);
                break;
            }
            case WeaponType::Triple: {
                auto bullet1 = std::make_shared<Bullet>("assets/images/weapons/bullet2.png", 20.0f);
                auto bullet2 = std::make_shared<Bullet>("assets/images/weapons/bullet2.png", 20.0f);
                auto bullet3 = std::make_shared<Bullet>("assets/images/weapons/bullet2.png", 20.0f);
                bullet1->setPosition(getPosition().x, getPosition().y - 30.f);
                bullet2->setPosition(getPosition().x - 25.f, getPosition().y - 15.f);
                bullet3->setPosition(getPosition().x + 25.f, getPosition().y - 15.f);
                newBullets.push_back(bullet1);
                newBullets.push_back(bullet2);
                newBullets.push_back(bullet3);
                break;
            }
            case WeaponType::Laser:
                // Laser is handled separately
                break;
        }
        
        return newBullets;
    }

    std::shared_ptr<Laser> shootLaser() {
        if (weaponType == WeaponType::Laser) {
            auto laser = std::make_shared<Laser>();
            laser->setPosition(getPosition().x, getPosition().y - 300.f);
            return laser;
        }
        return nullptr;
    }

    void takeDamage(int amount) {
        if (shield->isActive()) {
            shield->takeDamage(amount);
            return;
        }
        
        health -= amount;
        if (health < 0) health = 0;
    }

    void heal(int amount) {
        health += amount;
        if (health > 100) health = 100;
    }

    void activateShield() {
        shield->activate();
    }

    void upgradeWeapon() {
        switch (weaponType) {
            case WeaponType::Basic:
                weaponType = WeaponType::Double;
                break;
            case WeaponType::Double:
                weaponType = WeaponType::Triple;
                break;
            case WeaponType::Triple:
                weaponType = WeaponType::Laser;
                break;
            case WeaponType::Laser:
                // Already at max level
                addScore(50); // Give bonus points instead
                break;
        }
    }

    int getHealth() const { return health; }
    int getScore() const { return score; }
    void addScore(int points) { score += points; }
    void resetScore() { score = 0; }
    void resetHealth() { health = 100; }
    WeaponType getWeaponType() const { return weaponType; }
    void resetWeapon() { weaponType = WeaponType::Basic; }
    bool hasShield() const { return shield->isActive(); }
    float getShieldHealth() const { return shield->getHealth(); }
    void drawShield(sf::RenderWindow& window) const {
        if (shield->isActive()) {
            shield->draw(window);
        }
    }

private:
    float speed = 300.0f;
    int health;
    int score;
    WeaponType weaponType;
    sf::Clock shootClock;
    bool shieldActive;
    std::unique_ptr<Shield> shield;
};

// Enemy base class
class Enemy : public Entity {
public:
    Enemy(const std::string& texturePath, EnemyType type, float health, float speed, int scoreValue)
        : Entity(texturePath), type(type), health(health), speed(speed), scoreValue(scoreValue) {
        setScale(0.5f, 0.5f);
        setRotation(180.f); // Flip enemy to face down
    }

    virtual void update(float deltaTime) override {
        move(0.f, speed * deltaTime);
    }

    bool isOffScreen() const {
        return getPosition().y > 600;
    }

    void takeDamage(float amount) {
        health -= amount;
    }

    bool isDestroyed() const {
        return health <= 0;
    }

    EnemyType getType() const { return type; }
    int getScoreValue() const { return scoreValue; }

protected:
    EnemyType type;
    float health;
    float speed;
    int scoreValue;
};

// Basic enemy - moves straight down
class BasicEnemy : public Enemy {
public:
    BasicEnemy() : Enemy("assets/images/enemies/enemy1.png", EnemyType::Basic, 20.0f, 150.0f, 10) {}
};

// Fast enemy - moves faster but has less health
class FastEnemy : public Enemy {
public:
    FastEnemy() : Enemy("assets/images/enemies/enemy2.png", EnemyType::Fast, 10.0f, 250.0f, 15) {}
};

// Tanky enemy - moves slower but has more health
class TankyEnemy : public Enemy {
public:
    TankyEnemy() : Enemy("assets/images/enemies/enemy3.png", EnemyType::Tanky, 40.0f, 100.0f, 20) {}
};

// Boss enemy - special enemy with unique behavior
class BossEnemy : public Enemy {
public:
    BossEnemy() : Enemy("assets/images/enemies/boss.png", EnemyType::Boss, 500.0f, 50.0f, 500),
                  state(BossState::Entering), stateTime(0.0f), shootCooldown(0.0f) {
        setScale(1.0f, 1.0f);
    }

    void update(float deltaTime) override {
        stateTime += deltaTime;
        shootCooldown -= deltaTime;

        switch (state) {
            case BossState::Entering:
                // Move down to position
                if (getPosition().y < 100) {
                    move(0.f, speed * deltaTime);
                } else {
                    state = BossState::MovingLeft;
                    stateTime = 0.0f;
                }
                break;
                
            case BossState::MovingLeft:
                move(-speed * 1.5f * deltaTime, 0.f);
                if (stateTime > 2.0f || getPosition().x < 100) {
                    state = BossState::MovingRight;
                    stateTime = 0.0f;
                }
                break;
                
            case BossState::MovingRight:
                move(speed * 1.5f * deltaTime, 0.f);
                if (stateTime > 2.0f || getPosition().x > 700) {
                    state = BossState::MovingLeft;
                    stateTime = 0.0f;
                }
                break;
        }
    }

    bool canShoot() {
        if (shootCooldown <= 0.0f) {
            shootCooldown = 0.5f; // Shoot every 0.5 seconds
            return true;
        }
        return false;
    }

    std::vector<std::shared_ptr<Bullet>> shoot() {
        std::vector<std::shared_ptr<Bullet>> bullets;
        
        // Boss shoots 3 bullets in a spread pattern
        for (int i = -1; i <= 1; ++i) {
            auto bullet = std::make_shared<Bullet>("assets/images/weapons/bullet2.png");
            bullet->setPosition(getPosition().x + i * 30.f, getPosition().y + 50.f);
            bullet->setRotation(180.f); // Bullets go down
            bullets.push_back(bullet);
        }
        
        return bullets;
    }

private:
    enum class BossState {
        Entering,
        MovingLeft,
        MovingRight
    };
    
    BossState state;
    float stateTime;
    float shootCooldown;
};

// PowerUp class
class PowerUp : public Entity {
public:
    PowerUp(PowerUpType type) : Entity(getTexturePath(type)), type(type) {
        setScale(0.5f, 0.5f);
    }

    void update(float deltaTime) override {
        move(0.f, speed * deltaTime);
    }

    bool isOffScreen() const {
        return getPosition().y > 600;
    }

    PowerUpType getType() const { return type; }

private:
    std::string getTexturePath(PowerUpType type) {
        switch (type) {
            case PowerUpType::Health: return "assets/images/powerup.png";
            case PowerUpType::Shield: return "assets/images/effects/shield.png";
            case PowerUpType::WeaponUpgrade: return "assets/images/weapons/bullet2.png";
            case PowerUpType::ScoreBoost: return "assets/images/powerup.png";
            default: return "assets/images/powerup.png";
        }
    }

    PowerUpType type;
    float speed = 150.0f;
};

// Level system
class Level {
public:
    Level() : currentLevel(1), enemiesDefeated(0), bossSpawned(false) {}

    void update(int defeatedEnemies) {
        enemiesDefeated += defeatedEnemies;
        
        // Level up after defeating certain number of enemies
        if (enemiesDefeated >= getEnemiesForNextLevel() && !bossSpawned) {
            currentLevel++;
            enemiesDefeated = 0;
            bossSpawned = true;
        }
    }

    int getCurrentLevel() const { return currentLevel; }
    bool isBossLevel() const { return bossSpawned; }
    void resetBossFlag() { bossSpawned = false; }
    
    float getEnemySpawnInterval() const {
        return std::max(1.5f - (currentLevel - 1) * 0.1f, 0.5f);
    }
    
    float getPowerUpSpawnInterval() const {
        return std::max(10.0f - (currentLevel - 1) * 0.5f, 5.0f);
    }
    
    int getEnemiesForNextLevel() const {
        return 20 + (currentLevel - 1) * 5;
    }
    
    void reset() {
        currentLevel = 1;
        enemiesDefeated = 0;
        bossSpawned = false;
    }

private:
    int currentLevel;
    int enemiesDefeated;
    bool bossSpawned;
};

// Game class to manage the game state
class Game {
public:
    Game() : window(sf::VideoMode(800, 600), "Space Shooter"), gameState(GameState::MainMenu), deltaTime(0.0f) {
        window.setFramerateLimit(60);
        
        // Load resources
        loadResources();
        
        // Initialize game objects
        player.setPosition(400.f, 550.f);
        
        // Initialize UI elements
        initializeUI();
    }
    
    void run() {
        sf::Clock clock;
        
        while (window.isOpen()) {
            deltaTime = clock.restart().asSeconds();
            
            handleEvents();
            update();
            render();
        }
    }

private:
    void loadResources() {
        // Load font
        if (!font.loadFromFile("assets/arial.ttf")) {
            // Handle error
        }
        
        // Load sounds
        if (!shootBuffer.loadFromFile("assets/sounds/shoot.wav") ||
            !explosionBuffer.loadFromFile("assets/sounds/explosion.wav") ||
            !powerupBuffer.loadFromFile("assets/sounds/powerup.wav") ||
            !upgradeBuffer.loadFromFile("assets/sounds/upgrade.wav") ||
            !bossBuffer.loadFromFile("assets/sounds/boss.wav")) {
            // Handle error
        }
        
        shootSound.setBuffer(shootBuffer);
        explosionSound.setBuffer(explosionBuffer);
        powerupSound.setBuffer(powerupBuffer);
        upgradeSound.setBuffer(upgradeBuffer);
        bossSound.setBuffer(bossBuffer);
        
        // Load background
        if (!backgroundTexture.loadFromFile("assets/images/background.jpg")) {
            // Handle error
        }
        background.setTexture(backgroundTexture);
        
        // Scale background to fit window
        float scaleX = 800.0f / backgroundTexture.getSize().x;
        float scaleY = 600.0f / backgroundTexture.getSize().y;
        background.setScale(scaleX, scaleY);
        
        // Load explosion texture
        if (!explosionTexture.loadFromFile("assets/images/effects/explosion.png")) {
            // Handle error
        }
    }
    
    void initializeUI() {
        // Score text
        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10.f, 10.f);
        
        // Level text
        levelText.setFont(font);
        levelText.setCharacterSize(24);
        levelText.setFillColor(sf::Color::White);
        levelText.setPosition(10.f, 40.f);
        
        // Health bar
        healthBarBackground.setSize(sf::Vector2f(200.f, 20.f));
        healthBarBackground.setFillColor(sf::Color(100, 100, 100));
        healthBarBackground.setPosition(10.f, 70.f);
        
        healthBar.setSize(sf::Vector2f(200.f, 20.f));
        healthBar.setFillColor(sf::Color::Green);
        healthBar.setPosition(10.f, 70.f);
        
        // Shield bar
        shieldBarBackground.setSize(sf::Vector2f(200.f, 10.f));
        shieldBarBackground.setFillColor(sf::Color(100, 100, 100));
        shieldBarBackground.setPosition(10.f, 95.f);
        
        shieldBar.setSize(sf::Vector2f(200.f, 10.f));
        shieldBar.setFillColor(sf::Color::Cyan);
        shieldBar.setPosition(10.f, 95.f);
        
        // Weapon indicator
        weaponText.setFont(font);
        weaponText.setCharacterSize(18);
        weaponText.setFillColor(sf::Color::White);
        weaponText.setPosition(10.f, 110.f);
        
        // Game over text
        gameOverText.setFont(font);
        gameOverText.setCharacterSize(64);
        gameOverText.setFillColor(sf::Color::Red);
        gameOverText.setString("GAME OVER");
        gameOverText.setPosition(200.f, 200.f);
        
        // Victory text
        victoryText.setFont(font);
        victoryText.setCharacterSize(64);
        victoryText.setFillColor(sf::Color::Green);
        victoryText.setString("VICTORY!");
        victoryText.setPosition(250.f, 200.f);
        
        // Restart text
        restartText.setFont(font);
        restartText.setCharacterSize(32);
        restartText.setFillColor(sf::Color::White);
        restartText.setString("Press R to restart");
        restartText.setPosition(275.f, 300.f);
        
        // Main menu text
        titleText.setFont(font);
        titleText.setCharacterSize(64);
        titleText.setFillColor(sf::Color::Yellow);
        titleText.setString("SPACE SHOOTER");
        titleText.setPosition(150.f, 100.f);
        
        startText.setFont(font);
        startText.setCharacterSize(32);
        startText.setFillColor(sf::Color::White);
        startText.setString("Press ENTER to start");
        startText.setPosition(250.f, 300.f);
        
        controlsText.setFont(font);
        controlsText.setCharacterSize(24);
        controlsText.setFillColor(sf::Color::White);
        controlsText.setString("Controls:\nArrow Keys - Move\nSpace - Shoot");
        controlsText.setPosition(250.f, 400.f);
        
        // Boss warning text
        bossWarningText.setFont(font);
        bossWarningText.setCharacterSize(48);
        bossWarningText.setFillColor(sf::Color::Red);
        bossWarningText.setString("WARNING: BOSS APPROACHING!");
        bossWarningText.setPosition(75.f, 250.f);
        bossWarningVisible = false;
        bossWarningTime = 0.0f;
    }
    
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            
            // Handle key presses
            if (event.type == sf::Event::KeyPressed) {
                if (gameState == GameState::MainMenu && event.key.code == sf::Keyboard::Return) {
                    startGame();
                }
                else if ((gameState == GameState::GameOver || gameState == GameState::Victory) && 
                         event.key.code == sf::Keyboard::R) {
                    startGame();
                }
            }
        }
    }
    
    void update() {
        switch (gameState) {
            case GameState::MainMenu:
                // Nothing to update in main menu
                break;
                
            case GameState::Playing:
                updatePlaying();
                break;
                
            case GameState::BossFight:
                updateBossFight();
                break;
                
            case GameState::GameOver:
            case GameState::Victory:
                // Update explosions
                updateExplosions();
                break;
        }
    }
    
    void updatePlaying() {
        // Update player
        player.update(deltaTime);
        
        // Shooting
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && player.canShoot()) {
            if (player.getWeaponType() == WeaponType::Laser) {
                auto laser = player.shootLaser();
                if (laser) {
                    lasers.push_back(laser);
                    shootSound.play();
                }
            } else {
                auto newBullets = player.shoot();
                bullets.insert(bullets.end(), newBullets.begin(), newBullets.end());
                shootSound.play();
            }
        }
        
        // Spawn enemies
        enemySpawnTimer += deltaTime;
        if (enemySpawnTimer >= level.getEnemySpawnInterval()) {
            spawnEnemy();
            enemySpawnTimer = 0.0f;
        }
        
        // Spawn power-ups
        powerupSpawnTimer += deltaTime;
        if (powerupSpawnTimer >= level.getPowerUpSpawnInterval()) {
            spawnPowerUp();
            powerupSpawnTimer = 0.0f;
        }
        
        // Update bullets
        updateBullets();
        
        // Update lasers
        updateLasers();
        
        // Update enemies
        updateEnemies();
        
        // Update power-ups
        updatePowerUps();
        
        // Update explosions
        updateExplosions();
        
        // Check if it's time to spawn a boss
        if (level.isBossLevel()) {
            startBossFight();
        }
        
        // Update UI
        updateUI();
    }
    
    void updateBossFight() {
        // Update player
        player.update(deltaTime);
        
        // Shooting
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && player.canShoot()) {
            if (player.getWeaponType() == WeaponType::Laser) {
                auto laser = player.shootLaser();
                if (laser) {
                    lasers.push_back(laser);
                    shootSound.play();
                }
            } else {
                auto newBullets = player.shoot();
                bullets.insert(bullets.end(), newBullets.begin(), newBullets.end());
                shootSound.play();
            }
        }
        
        // Update boss
        if (!boss) {
            // Create boss if it doesn't exist
            boss = std::make_shared<BossEnemy>();
            boss->setPosition(400.f, -50.f);
            bossSound.play();
        } else {
            boss->update(deltaTime);
            
            // Boss shooting
            if (boss->canShoot()) {
                auto bossBullets = boss->shoot();
                enemyBullets.insert(enemyBullets.end(), bossBullets.begin(), bossBullets.end());
            }
            
            // Check if boss is destroyed
            if (boss->isDestroyed()) {
                // Create explosion
                explosions.emplace_back(boss->getPosition(), 2.0f);
                explosionSound.play();
                
                // Add score
                player.addScore(boss->getScoreValue());
                
                // Clear boss
                boss = nullptr;
                
                // Reset boss flag
                level.resetBossFlag();
                
                // Check if this was the final boss (level 5)
                if (level.getCurrentLevel() >= 5) {
                    gameState = GameState::Victory;
                } else {
                    gameState = GameState::Playing;
                }
            }
        }
        
        // Update bullets
        updateBullets();
        
        // Update enemy bullets
        for (auto it = enemyBullets.begin(); it != enemyBullets.end();) {
            (*it)->move(0.f, 300.0f * deltaTime); // Enemy bullets move down
            
            // Check collision with player
            if ((*it)->getBounds().intersects(player.getBounds())) {
                player.takeDamage(10);
                it = enemyBullets.erase(it);
                
                // Check if player is dead
                if (player.getHealth() <= 0) {
                    gameState = GameState::GameOver;
                    explosions.emplace_back(player.getPosition());
                    explosionSound.play();
                }
            }
            // Remove off-screen bullets
            else if ((*it)->getPosition().y > 600) {
                it = enemyBullets.erase(it);
            } else {
                ++it;
            }
        }
        
        // Update lasers
        updateLasers();
        
        // Update explosions
        updateExplosions();
        
        // Update UI
        updateUI();
        
        // Update boss warning
        if (bossWarningVisible) {
            bossWarningTime += deltaTime;
            if (bossWarningTime >= 3.0f) {
                bossWarningVisible = false;
            }
        }
    }
    
    void updateBullets() {
        for (auto it = bullets.begin(); it != bullets.end();) {
            (*it)->update(deltaTime);
            
            bool bulletRemoved = false;
            
            // Check collision with enemies
            for (auto enemyIt = enemies.begin(); enemyIt != enemies.end() && !bulletRemoved;) {
                if ((*it)->getBounds().intersects((*enemyIt)->getBounds())) {
                    // Enemy hit
                    (*enemyIt)->takeDamage((*it)->getDamage());
                    
                    if ((*enemyIt)->isDestroyed()) {
                        // Create explosion
                        explosions.emplace_back((*enemyIt)->getPosition());
                        explosionSound.play();
                        
                        // Add score
                        player.addScore((*enemyIt)->getScoreValue());
                        
                        // Update level
                        level.update(1);
                        
                        // Remove enemy
                        enemyIt = enemies.erase(enemyIt);
                    } else {
                        ++enemyIt;
                    }
                    
                    // Remove bullet
                    it = bullets.erase(it);
                    bulletRemoved = true;
                } else {
                    ++enemyIt;
                }
            }
            
            // Check collision with boss
            if (!bulletRemoved && boss && (*it)->getBounds().intersects(boss->getBounds())) {
                boss->takeDamage((*it)->getDamage());
                it = bullets.erase(it);
                bulletRemoved = true;
            }
            
            // Remove off-screen bullets
            if (!bulletRemoved) {
                if ((*it)->isOffScreen()) {
                    it = bullets.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    
    void updateLasers() {
        for (auto it = lasers.begin(); it != lasers.end();) {
            (*it)->update(deltaTime);
            
            // Check collision with enemies
            for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
                if ((*it)->getBounds().intersects((*enemyIt)->getBounds())) {
                    // Enemy hit by laser
                    (*enemyIt)->takeDamage((*it)->getDamage());
                    
                    if ((*enemyIt)->isDestroyed()) {
                        // Create explosion
                        explosions.emplace_back((*enemyIt)->getPosition());
                        explosionSound.play();
                        
                        // Add score
                        player.addScore((*enemyIt)->getScoreValue());
                        
                        // Update level
                        level.update(1);
                        
                        // Remove enemy
                        enemyIt = enemies.erase(enemyIt);
                    } else {
                        ++enemyIt;
                    }
                } else {
                    ++enemyIt;
                }
            }
            
            // Check collision with boss
            if (boss && (*it)->getBounds().intersects(boss->getBounds())) {
                boss->takeDamage((*it)->getDamage());
            }
            
            // Remove inactive lasers
            if (!(*it)->isActive()) {
                it = lasers.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void updateEnemies() {
        for (auto it = enemies.begin(); it != enemies.end();) {
            (*it)->update(deltaTime);
            
            // Check collision with player
            if ((*it)->getBounds().intersects(player.getBounds())) {
                // Player hit by enemy
                player.takeDamage(25);
                explosionSound.play();
                explosions.emplace_back((*it)->getPosition());
                it = enemies.erase(it);
                
                // Check if player is dead
                if (player.getHealth() <= 0) {
                    gameState = GameState::GameOver;
                    explosions.emplace_back(player.getPosition());
                }
            }
            // Remove off-screen enemies
            else if ((*it)->isOffScreen()) {
                it = enemies.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void updatePowerUps() {
        for (auto it = powerups.begin(); it != powerups.end();) {
            (*it)->update(deltaTime);
            
            // Check collision with player
            if ((*it)->getBounds().intersects(player.getBounds())) {
                // Apply power-up effect
                switch ((*it)->getType()) {
                    case PowerUpType::Health:
                        player.heal(25);
                        powerupSound.play();
                        break;
                        
                    case PowerUpType::Shield:
                        player.activateShield();
                        powerupSound.play();
                        break;
                        
                    case PowerUpType::WeaponUpgrade:
                        player.upgradeWeapon();
                        upgradeSound.play();
                        break;
                        
                    case PowerUpType::ScoreBoost:
                        player.addScore(50);
                        powerupSound.play();
                        break;
                }
                
                it = powerups.erase(it);
            }
            // Remove off-screen power-ups
            else if ((*it)->isOffScreen()) {
                it = powerups.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void updateExplosions() {
        for (auto it = explosions.begin(); it != explosions.end();) {
            if (!it->update(deltaTime)) {
                it = explosions.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void updateUI() {
        // Update score text
        scoreText.setString("Score: " + std::to_string(player.getScore()));
        
        // Update level text
        levelText.setString("Level: " + std::to_string(level.getCurrentLevel()));
        
        // Update health bar
        float healthPercent = static_cast<float>(player.getHealth()) / 100.0f;
        healthBar.setSize(sf::Vector2f(200.f * healthPercent, 20.f));
        
        // Change health bar color based on health
        if (healthPercent > 0.6f) {
            healthBar.setFillColor(sf::Color::Green);
        } else if (healthPercent > 0.3f) {
            healthBar.setFillColor(sf::Color::Yellow);
        } else {
            healthBar.setFillColor(sf::Color::Red);
        }
        
        // Update shield bar
        if (player.hasShield()) {
            float shieldPercent = player.getShieldHealth() / 100.0f;
            shieldBar.setSize(sf::Vector2f(200.f * shieldPercent, 10.f));
            shieldBar.setFillColor(sf::Color::Cyan);
        } else {
            shieldBar.setSize(sf::Vector2f(0.f, 10.f));
        }
        
        // Update weapon text
        std::string weaponName;
        switch (player.getWeaponType()) {
            case WeaponType::Basic: weaponName = "Basic"; break;
            case WeaponType::Double: weaponName = "Double"; break;
            case WeaponType::Triple: weaponName = "Triple"; break;
            case WeaponType::Laser: weaponName = "Laser"; break;
        }
        weaponText.setString("Weapon: " + weaponName);
    }
    
    void spawnEnemy() {
        std::shared_ptr<Enemy> enemy;
        
        // Random enemy type based on level
        int maxEnemyType = std::min(3, level.getCurrentLevel());
        int enemyType = rand() % maxEnemyType;
        
        switch (enemyType) {
            case 0:
                enemy = std::make_shared<BasicEnemy>();
                break;
            case 1:
                enemy = std::make_shared<FastEnemy>();
                break;
            case 2:
                enemy = std::make_shared<TankyEnemy>();
                break;
        }
        
        float x = static_cast<float>(rand() % 750 + 25);
        enemy->setPosition(x, -50.f);
        enemies.push_back(enemy);
    }
    
    void spawnPowerUp() {
        // Random power-up type
        PowerUpType type = static_cast<PowerUpType>(rand() % 4);
        
        auto powerup = std::make_shared<PowerUp>(type);
        float x = static_cast<float>(rand() % 750 + 25);
        powerup->setPosition(x, -50.f);
        powerups.push_back(powerup);
    }
    
    void startBossFight() {
        gameState = GameState::BossFight;
        bossWarningVisible = true;
        bossWarningTime = 0.0f;
    }
    
    void startGame() {
        // Reset game state
        gameState = GameState::Playing;
        
        // Reset player
        player.resetHealth();
        player.resetScore();
        player.resetWeapon();
        player.setPosition(400.f, 550.f);
        
        // Clear game objects
        bullets.clear();
        lasers.clear();
        enemies.clear();
        powerups.clear();
        explosions.clear();
        enemyBullets.clear();
        boss = nullptr;
        
        // Reset level
        level.reset();
        
        // Reset timers
        enemySpawnTimer = 0.0f;
        powerupSpawnTimer = 0.0f;
    }
    
    void render() {
        window.clear();
        
        // Draw background
        window.draw(background);
        
        // Draw based on game state
        switch (gameState) {
            case GameState::MainMenu:
                renderMainMenu();
                break;
                
            case GameState::Playing:
            case GameState::BossFight:
                renderGame();
                break;
                
            case GameState::GameOver:
                renderGame();
                window.draw(gameOverText);
                window.draw(restartText);
                break;
                
            case GameState::Victory:
                renderGame();
                window.draw(victoryText);
                window.draw(restartText);
                break;
        }
        
        window.display();
    }
    
    void renderMainMenu() {
        window.draw(titleText);
        window.draw(startText);
        window.draw(controlsText);
    }
    
    void renderGame() {
        // Draw player
        player.draw(window);
        
        // Draw player shield
        player.drawShield(window);
        
        // Draw bullets
        for (const auto& bullet : bullets) {
            bullet->draw(window);
        }
        
        // Draw enemy bullets
        for (const auto& bullet : enemyBullets) {
            bullet->draw(window);
        }
        
        // Draw lasers
        for (const auto& laser : lasers) {
            laser->draw(window);
        }
        
        // Draw enemies
        for (const auto& enemy : enemies) {
            enemy->draw(window);
        }
        
        // Draw boss
        if (boss) {
            boss->draw(window);
        }
        
        // Draw power-ups
        for (const auto& powerup : powerups) {
            powerup->draw(window);
        }
        
        // Draw explosions
        for (const auto& explosion : explosions) {
            explosion.draw(window);
        }
        
        // Draw UI
        window.draw(scoreText);
        window.draw(levelText);
        window.draw(healthBarBackground);
        window.draw(healthBar);
        window.draw(shieldBarBackground);
        window.draw(shieldBar);
        window.draw(weaponText);
        
        // Draw boss warning
        if (bossWarningVisible) {
            window.draw(bossWarningText);
        }
    }
    
    // Game window
    sf::RenderWindow window;
    
    // Game state
    GameState gameState;
    
    // Delta time for frame-rate independent movement
    float deltaTime;
    
    // Resources
    sf::Font font;
    sf::Texture backgroundTexture;
    sf::Sprite background;
    sf::Texture explosionTexture;
    
    // Sounds
    sf::SoundBuffer shootBuffer, explosionBuffer, powerupBuffer, upgradeBuffer, bossBuffer;
    sf::Sound shootSound, explosionSound, powerupSound, upgradeSound, bossSound;
    
    // Game objects
    Player player;
    std::vector<std::shared_ptr<Bullet>> bullets;
    std::vector<std::shared_ptr<Laser>> lasers;
    std::vector<std::shared_ptr<Enemy>> enemies;
    std::vector<std::shared_ptr<PowerUp>> powerups;
    std::vector<std::shared_ptr<Bullet>> enemyBullets;
    std::shared_ptr<BossEnemy> boss;
    std::vector<Explosion> explosions;
    
    // Level system
    Level level;
    
    // Timers
    float enemySpawnTimer = 0.0f;
    float powerupSpawnTimer = 0.0f;
    
    // UI elements
    sf::Text scoreText;
    sf::Text levelText;
    sf::RectangleShape healthBarBackground;
    sf::RectangleShape healthBar;
    sf::RectangleShape shieldBarBackground;
    sf::RectangleShape shieldBar;
    sf::Text weaponText;
    sf::Text gameOverText;
    sf::Text victoryText;
    sf::Text restartText;
    sf::Text titleText;
    sf::Text startText;
    sf::Text controlsText;
    sf::Text bossWarningText;
    bool bossWarningVisible;
    float bossWarningTime;
};

int main() {
    // Initialize random seed
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // Create and run the game
    Game game;
    game.run();
    
    return 0;
}
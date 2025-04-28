#include "confetticontroller.h"
#include <QElapsedTimer>
#include "chessboard.h"
#include <QPainter>
#include <QTimer>
#include <random>

// color palette for particles
const std::vector<QColor> ConfettiController::kPalette = {
    QColor(0x0079FF), QColor(0x00DFA2), QColor(0xF6FA70),
    QColor(0x0079FF), QColor(0x00DFA2), QColor(0xFF0060)
};

ConfettiController::ConfettiController(ChessBoard* board, QObject* parent)
    : QObject(parent),
    world(b2Vec2(0.0f, -0.5f)),   // gravity
    timer(this),
    board(board)
{
    m_clock.start();
    connect(&timer, &QTimer::timeout, this, &ConfettiController::stepPhysics);
    timer.start(16);  // ~60 Hz
}

ConfettiController::~ConfettiController() {
    for (auto &p : m_parts)
        world.DestroyBody(p.body);
}

void ConfettiController::spawn(int count) {
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> xDist(0.0f, 8.0f);
    std::uniform_real_distribution<float> sizeDist(0.03f, 0.08f);
    std::uniform_real_distribution<float> angDist(-3.0f, 3.0f);
    std::uniform_int_distribution<int> colDist(0, int(kPalette.size()) - 1);
    float now = m_clock.elapsed() * 0.001f;

    for (int i = 0; i < count; ++i) {
        b2BodyDef body{ };
        body.type = b2_dynamicBody;
        body.position.Set(xDist(rng), 12.0f);
        b2Body* object_body = world.CreateBody(&body);

        float half = sizeDist(rng);
        b2PolygonShape box; box.SetAsBox(half, half);
        b2FixtureDef fix;
        fix.shape = &box;
        fix.density = 1.0f;
        fix.restitution = 0.4f + 0.2f * (rng() % 100 / 100.0f);
        object_body->CreateFixture(&fix);

        object_body->SetLinearVelocity({ (xDist(rng) - 4.0f), -2.0f });
        object_body->SetAngularVelocity(angDist(rng));
        object_body->SetLinearDamping(1.5f);
        object_body->SetAngularDamping(0.3f);

        Particles p{ object_body, kPalette[colDist(rng)], half, now };
        m_parts.push_back(p);
    }
}

void ConfettiController::spawnAt(const b2Vec2& pos, int count) {
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> sizeDist(0.03f, 0.08f);
    std::uniform_real_distribution<float> angDist(-3.0f, 3.0f);
    std::uniform_int_distribution<int> colDist(0, int(kPalette.size()) - 1);
    float now = m_clock.elapsed() * 0.001f;

    for (int i = 0; i < count; ++i) {
        b2BodyDef bd{ };
        bd.type = b2_dynamicBody;
        bd.position = pos;
        bd.bullet = true;
        b2Body* b = world.CreateBody(&bd);

        float half = sizeDist(rng);
        b2PolygonShape box; box.SetAsBox(half, half);
        b2FixtureDef fd{ };
        fd.shape = &box;
        fd.density = 1.0f;
        fd.restitution = 0.6f;
        fd.friction = 0.2f;
        b->CreateFixture(&fd);

        float angle = std::uniform_real_distribution<float>(0, 2*b2_pi)(rng);
        b2Vec2 impulse = 5.0f * b2Vec2(cosf(angle), sinf(angle));
        b->ApplyLinearImpulse(impulse, b->GetWorldCenter(), true);

        Particles p{ b, kPalette[colDist(rng)], half, now };
        m_parts.push_back(p);
    }
}

void ConfettiController::stepPhysics() {
    if (m_parts.empty()) return;

    world.Step(1.0f/60.0f, 8, 3);

    const float worldW = 8.0f;
    for (auto &p : m_parts) {
        b2Body* b = p.body;
        b2Vec2 pos = b->GetPosition();
        b2Vec2 vel = b->GetLinearVelocity();
        if (pos.x < 0.0f || pos.x > worldW) {
            pos.x = qBound(0.0f, pos.x, worldW);
            vel.x = (pos.x < 0.1f ? +std::abs(vel.x) : -std::abs(vel.x));
            b->SetTransform(pos, b->GetAngle());
            b->SetLinearVelocity(vel);
        }
    }

    float now = m_clock.elapsed() * 0.001f;
    for (int i = int(m_parts.size())-1; i >= 0; --i) {
        float age = now - m_parts[i].birthTime;
        if (age > (kLifeSpan + kFadeDuration)) {
            world.DestroyBody(m_parts[i].body);
            m_parts.erase(m_parts.begin()+i);
        }
    }

    board->update();
}

void ConfettiController::draw(QPainter& painter) const {
    float now = m_clock.elapsed() * 0.001f;
    for (auto const& p : m_parts) {
        float age = now - p.birthTime;
        float alpha = (age > kLifeSpan)
                          ? qMax(0.0f, 1.0f - (age - kLifeSpan)/kFadeDuration)
                          : 1.0f;
        painter.setOpacity(alpha);

        b2Vec2 pos = p.body->GetPosition();
        float px = pos.x * kScale;
        float py = (8.0f*kScale) - (pos.y*kScale);

        painter.save();
        painter.translate(px, py);
        painter.rotate(p.body->GetAngle() * 180.0f / b2_pi);
        float sidePx = p.size * 2 * kScale;
        QRectF r(-sidePx/2, -sidePx/2, sidePx, sidePx);
        painter.fillRect(r, p.color);
        painter.restore();
    }
    painter.setOpacity(1.0f);
}

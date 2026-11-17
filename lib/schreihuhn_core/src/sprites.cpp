#include "schreihuhn/render/sprites.h"

namespace schreihuhn::render {

namespace {

constexpr uint8_t charToIdx(char c) {
    if (c == '.') return K_TRANSPARENT;
    if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(10 + (c - 'A'));
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(10 + (c - 'a'));
    return K_TRANSPARENT;
}

template <std::size_t W, std::size_t H>
struct SpriteData {
    std::array<uint8_t, W * H> px{};
};

template <std::size_t W, std::size_t H>
constexpr SpriteData<W, H> makeSprite(const char* const (&rows)[H]) {
    SpriteData<W, H> s{};
    for (std::size_t y = 0; y < H; ++y) {
        for (std::size_t x = 0; x < W; ++x) {
            s.px[y * W + x] = charToIdx(rows[y][x]);
        }
    }
    return s;
}

// --- Chicken 16x16 sprites (hitbox 10x13 = x:3..12, y:3..15) ---
// Slots: 0=outline/eye, 7=body, 6=shadow, 8=comb, 9=beak/feet, E=cheek, F=glint
constexpr SpriteData<16, 16> kChickenIdle0 = makeSprite<16, 16>({
    ".......000......",
    "......08880.....",
    "......0880......",
    ".....000000.....",
    "....07777770....",
    "...077F770770...",
    "...07777777700..",
    "..07777777E7990.",
    "..076677777000..",
    "..0667777670....",
    "..066666660.....",
    "...0066660......",
    ".....0000.......",
    ".....09.09......",
    ".....09.09......",
    "....099.099....."
});

constexpr SpriteData<16, 16> kChickenIdle1 = makeSprite<16, 16>({
    "................",
    ".......000......",
    "......08880.....",
    "......08800.....",
    ".....0000770....",
    "....077770770...",
    "...077F7777700..",
    "...0777777E7990.",
    "..076677777000..",
    "..0667777670....",
    "..066666660.....",
    "...0066660......",
    ".....0000.......",
    ".....09.09......",
    ".....09.09......",
    "....099.099....."
});

constexpr SpriteData<16, 16> kChickenWalk0 = makeSprite<16, 16>({
    ".......000......",
    "......08880.....",
    "......0880......",
    ".....000000.....",
    "....07777770....",
    "...077F770770...",
    "...07777777700..",
    "..07777777E7990.",
    "..076677777000..",
    "..0667777670....",
    "..066666660.....",
    "...0066660......",
    ".....0000.......",
    "....09..09......",
    "...09....09.....",
    "..099....099...."
});

constexpr SpriteData<16, 16> kChickenWalk1 = kChickenIdle0;

constexpr SpriteData<16, 16> kChickenWalk2 = makeSprite<16, 16>({
    "................",
    ".......000......",
    "......08880.....",
    "......08800.....",
    ".....0000770....",
    "....077770770...",
    "...077F7777700..",
    "...0777777E7990.",
    "..076677777000..",
    "..0667777670....",
    "..066666660.....",
    "...0066660......",
    ".....0000.......",
    "......0909......",
    "......09.09.....",
    ".....099..09...."
});

constexpr SpriteData<16, 16> kChickenWalk3 = kChickenIdle1;

constexpr SpriteData<16, 16> kChickenJump = makeSprite<16, 16>({
    ".......000......",
    "......08880.....",
    "......0880......",
    ".....000000.....",
    "....07777770....",
    "...077F770770...",
    "..007777777700..",
    ".077777777E7990.",
    ".0766777777000..",
    "..0666777670....",
    "...06666660.....",
    "....000000......",
    ".....09.09......",
    "....099.099.....",
    "................",
    "................"
});

constexpr SpriteData<16, 16> kChickenFlap0 = makeSprite<16, 16>({
    ".......000......",
    "......08880.....",
    "...00.0880......",
    "..077000000.....",
    ".07777777770....",
    ".07677F770770...",
    "..066777777700..",
    "...0777777E7990.",
    "...07677777000..",
    "...066677670....",
    "....0666660.....",
    ".....00000......",
    ".....09.09......",
    ".....099.099....",
    "................",
    "................"
});

constexpr SpriteData<16, 16> kChickenFlap1 = makeSprite<16, 16>({
    ".......000......",
    "......08880.....",
    "......0880......",
    ".....000000.....",
    "....07777770....",
    "...077F770770...",
    "...07777777700..",
    "..07777777E7990.",
    "..076677777000..",
    ".07666777670....",
    "07666666660.....",
    "000..00000......",
    ".....09.09......",
    ".....099.099....",
    "................",
    "................"
});

constexpr SpriteData<16, 16> kChickenFall = makeSprite<16, 16>({
    ".......000......",
    "......08880.....",
    "..00..0880......",
    ".0770000000.....",
    ".07777770070....",
    ".07677F707070...",
    "..066777007700..",
    "...0777777E7990.",
    "...07677777000..",
    "...066677670....",
    "....0666660.....",
    ".....00000......",
    "....09...09.....",
    "...099...099....",
    "................",
    "................"
});

// Ninja headband overlay (2 frames)
constexpr SpriteData<16, 16> kNinjaHeadband0 = makeSprite<16, 16>({
    "................",
    "................",
    "................",
    ".000............",
    "0888888888880...",
    ".080............",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................"
});

constexpr SpriteData<16, 16> kNinjaHeadband1 = makeSprite<16, 16>({
    "................",
    "................",
    ".00.............",
    "0880............",
    ".000888888880...",
    "0880............",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................"
});

// Remap tables
constexpr RemapTable kRemapHenni = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 7, 7
};

constexpr RemapTable kRemapGoldie = {
    0, 1, 2, 3, 4, 5, 9, 10, 8, 9, 10, 11, 12, 13, 14, 7
};

constexpr RemapTable kRemapNinja = {
    0, 1, 2, 3, 4, 5, 0, 5, 5, 9, 10, 11, 12, 13, 5, 5
};

constexpr RemapTable kRemapLocked = {
    0, 1, 1, 1, 1, 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

// --- Coin 8x8 (4 rotation frames) ---
constexpr SpriteData<8, 8> kCoin0 = makeSprite<8, 8>({
    "..0000..",
    ".0AAAA0.",
    "0AA77A90",
    "0AA7AA90",
    "0AA7AA90",
    "0AA99A90",
    ".099990.",
    "..0000.."
});

constexpr SpriteData<8, 8> kCoin1 = makeSprite<8, 8>({
    "..0000..",
    "..0AA0..",
    ".0A7A90.",
    ".0A7A90.",
    ".0A7A90.",
    ".0A9A90.",
    "..0990..",
    "..0000.."
});

constexpr SpriteData<8, 8> kCoin2 = makeSprite<8, 8>({
    "...00...",
    "...070..",
    "...0A0..",
    "...0A0..",
    "...0A0..",
    "...090..",
    "...090..",
    "...00..."
});

constexpr SpriteData<8, 8> kCoin3 = makeSprite<8, 8>({
    "..0000..",
    "..0AA0..",
    ".09A7A0.",
    ".09A7A0.",
    ".09A7A0.",
    ".09A9A0.",
    "..0990..",
    "..0000.."
});

// --- Coin Sparkle 8x8 (3 frames) ---
constexpr SpriteData<8, 8> kSparkle0 = makeSprite<8, 8>({
    "...7....",
    "...7....",
    ".7.A.7..",
    "77AAA77.",
    "...A....",
    ".7.7.7..",
    "...7....",
    "........"
});

constexpr SpriteData<8, 8> kSparkle1 = makeSprite<8, 8>({
    ".7...7..",
    "..E.E...",
    "...7....",
    "7.7A7.7.",
    "...7....",
    "..E.E...",
    ".7...7..",
    "........"
});

constexpr SpriteData<8, 8> kSparkle2 = makeSprite<8, 8>({
    "7.....7.",
    "........",
    "........",
    "........",
    "........",
    "........",
    "7.....7.",
    "........"
});

// --- Obstacles ---
constexpr SpriteData<8, 8> kHayBale = makeSprite<8, 8>({
    "00000000",
    "0AA9AAA0",
    "0A999AA0",
    "04444440",
    "0AA9AA90",
    "09AA9AA0",
    "09949940",
    "00000000"
});

constexpr SpriteData<8, 16> kFence = makeSprite<8, 16>({
    ".00..00.",
    "09400940",
    "04400440",
    "09999990",
    "04444440",
    "04400440",
    "04400440",
    "09999990",
    "04444440",
    "04400440",
    "04400440",
    "04400440",
    "04400440",
    "04400440",
    "04400440",
    "00000000"
});

constexpr SpriteData<16, 16> kStoneBlock = makeSprite<16, 16>({
    "0000000000000000",
    "0777766666666650",
    "0766666666566650",
    "0766655666666550",
    "0666666666665550",
    "0665666666666550",
    "0666666556666650",
    "0666666666666550",
    "0655666666556550",
    "0666666666666550",
    "0666656666666550",
    "0666666655665550",
    "0665566666665550",
    "0555666666555550",
    "0555555555555550",
    "0000000000000000"
});

constexpr SpriteData<8, 24> kTower = makeSprite<8, 24>({
    "00000000",
    "0AA9AAA0",
    "0A999AA0",
    "04444440",
    "0AA9AA90",
    "09AA9AA0",
    "09949940",
    "00000000",
    "0AA9AAA0",
    "0A999AA0",
    "04444440",
    "0AA9AA90",
    "09AA9AA0",
    "09949940",
    "00000000",
    "09444490",
    "04944940",
    "04499440",
    "04499440",
    "04944940",
    "09444490",
    "04444440",
    "04444440",
    "00000000"
});

// Overhang (24x12 floating obstacle with jagged underside teeth; remaps cleanly per biome)
constexpr SpriteData<24, 12> kOverhang = makeSprite<24, 12>({
    ".0000000000000000000000.",
    "0BBBBBBBB77BBBBBBBBBBBB0",
    "0B3BBBBBB33BBBBBBBB3BBB0",
    "03333BBBB333BBBBBB333BB0",
    "044444444444444444444440",
    "099499499499499499499490",
    "044444444444444444444440",
    "000330009003300090033000",
    "..030...0..030..0..030..",
    "..0A0.......00......00..",
    "...0....................",
    "........................"
});

// WideBarrier (24x16 solid wall / double barricade)
constexpr SpriteData<24, 16> kWideBarrier = makeSprite<24, 16>({
    "000000000000000000000000",
    "0AA9AA00AA9AA00AA9AA9AA0",
    "0A999A00A999A00A999A99A0",
    "044444004444400444444440",
    "099999999999999999999990",
    "044444444444444444444440",
    "0440AA9A00440AA9A0044000",
    "0440A99900440A9990044000",
    "099999999999999999999990",
    "044444444444444444444440",
    "044004400440044004400440",
    "094009400940094009400940",
    "044004400440044004400440",
    "044004400440044004400440",
    "044004400440044004400440",
    "000000000000000000000000"
});

// Pillar (16x24 tall stone monolith)
constexpr SpriteData<16, 24> kPillar = makeSprite<16, 24>({
    "0000000000000000",
    "0777776666666650",
    "0766666665666550",
    "0666556666665550",
    "0666666666655550",
    "0655666655666550",
    "0666666666665550",
    "0000000000000000",
    "0766666556666650",
    "0666656666665550",
    "0665566666556550",
    "0666666666665550",
    "0666665566666550",
    "0665666666665550",
    "0666666556665550",
    "0000000000000000",
    "0766665666666550",
    "0666666665565550",
    "0655666666666550",
    "0666665566665550",
    "0666566666655550",
    "0555666665555550",
    "0555555555555550",
    "0000000000000000"
});

// --- Background Decor Sprites (non-colliding world props) ---
constexpr SpriteData<12, 16> kDecorGreenTree = makeSprite<12, 16>({
    "...BBBB.....",
    "..BBBBBB....",
    ".BB7BBBBB...",
    ".BBBBBBBB3..",
    ".BBB3BBBB3..",
    "..B333333...",
    "....3333....",
    ".....44.....",
    ".....44.....",
    ".....44.....",
    ".....44.....",
    ".....44.....",
    ".....44.....",
    "....4444....",
    "............",
    "............"
});

constexpr SpriteData<8, 4> kDecorGrassTuft = makeSprite<8, 4>({
    ".B..B...",
    "B3.BB.B.",
    "33B33B3.",
    "3333333."
});

constexpr SpriteData<6, 6> kDecorFlower = makeSprite<6, 6>({
    ".88...",
    "8A8...",
    ".8..E.",
    ".3.E7E",
    "33..3.",
    ".3.33."
});

constexpr SpriteData<12, 18> kDecorSpruce = makeSprite<12, 18>({
    ".....3......",
    "....333.....",
    "...3B333....",
    "....333.....",
    "...33B33....",
    "..3333333...",
    "...33333....",
    "..33B3333...",
    ".333333333..",
    "..3333333...",
    ".333333333..",
    "33333333333.",
    "....333.....",
    ".....44.....",
    ".....44.....",
    ".....44.....",
    "....0440....",
    "............"
});

constexpr SpriteData<10, 18> kDecorBirch = makeSprite<10, 18>({
    "..BBBB....",
    ".BBABBB...",
    "BBBBBB3B..",
    "BB3BBBB3..",
    ".B3B333...",
    "....77....",
    "....07....",
    "....77....",
    "....70....",
    "....77....",
    "....07....",
    "....77....",
    "....70....",
    "....77....",
    "....77....",
    "....07....",
    "....77....",
    "...6776..."
});

constexpr SpriteData<10, 6> kDecorRock = makeSprite<10, 6>({
    "..6666....",
    ".676655...",
    "66655555..",
    "555555555.",
    "555555555.",
    "055555550."
});

constexpr SpriteData<12, 18> kDecorSnowSpruce = makeSprite<12, 18>({
    ".....7......",
    "....776.....",
    "...77766....",
    "....333.....",
    "...77333....",
    "..7766333...",
    "...33333....",
    "..7776333...",
    ".776633333..",
    "..3333333...",
    ".777633333..",
    "77633333333.",
    "....333.....",
    ".....44.....",
    ".....44.....",
    ".....44.....",
    "....6776....",
    "............"
});

constexpr SpriteData<10, 14> kDecorLavaSpire = makeSprite<10, 14>({
    "....8.....",
    "...950....",
    "...550....",
    "..55500...",
    "..59500...",
    "..58500...",
    ".5555500..",
    ".5595500..",
    ".5585500..",
    "555555000.",
    "595555000.",
    "585555000.",
    "555555000.",
    "000000000."
});

constexpr SpriteData<8, 8> kDecorLavaCrystal = makeSprite<8, 8>({
    "...9....",
    "..A9....",
    "..98.9..",
    ".58898..",
    ".558850.",
    "5555550.",
    "5985550.",
    "0000000."
});

// Jungle: tall multi-canopy tree (16x26), stepped temple ruin (22x16), fern (8x5)
constexpr SpriteData<16, 26> kDecorJungleTree = makeSprite<16, 26>({
    "....BBBBBBB.....",
    "..BB7BBBBBBB3...",
    ".BBBBBBBBBB333..",
    ".B33BBBB333333..",
    "...333.44.333...",
    ".BBBBBB44BBBB...",
    "BBBB33B44BB333..",
    "33333..44..333..",
    ".......44.......",
    "......444.......",
    "......404.......",
    "......444.......",
    "......443.......",
    "......444.......",
    "......404.......",
    "......444.......",
    ".....3444.......",
    "......444.......",
    "......440.......",
    "......444.......",
    "......444.......",
    ".....34443......",
    ".....44444......",
    "....4444444.....",
    "...044444440....",
    "................"
});

constexpr SpriteData<22, 16> kDecorJungleTemple = makeSprite<22, 16>({
    "........000000........",
    ".......06666550.......",
    ".......06335550.......",
    ".....000665555000.....",
    "....06666553355550....",
    "....06355555555550....",
    "..000665553355555000..",
    ".06665555555553355550.",
    ".06555500000000555550.",
    "0065550000000000553500",
    "0665350000000000555550",
    "0655550000000000555550",
    "0553550000000000535550",
    "0555550000000000555550",
    "0000000000000000000000",
    "......................"
});

constexpr SpriteData<8, 5> kDecorFern = makeSprite<8, 5>({
    ".B...B..",
    "BB.B.B3.",
    ".33B33..",
    "3B333B3.",
    "3333333."
});

// Sand: small cactus (8x10), tall saguaro cactus (10x18), desert sandstone rock (10x6)
constexpr SpriteData<8, 10> kDecorCactusSmall = makeSprite<8, 10>({
    "...8A...",
    "..0BB0..",
    "..0B30..",
    ".00B300.",
    "0BB33BB0",
    "03333330",
    ".00B300.",
    "..0B30..",
    "..0B30..",
    "..0000.."
});

constexpr SpriteData<10, 18> kDecorCactusTall = makeSprite<10, 18>({
    "...000....",
    "...0BB0...",
    "...0B30...",
    "00.0B30...",
    "0B00B3000.",
    "0B00B30B0.",
    "0300B30B0.",
    "033333030.",
    ".000B3330.",
    "...0B300..",
    "...0B30...",
    "...0B30...",
    "...0B30...",
    "...0B30...",
    "...0B30...",
    "...0B30...",
    "...0330...",
    "...0000..."
});

constexpr SpriteData<10, 6> kDecorDesertRock = makeSprite<10, 6>({
    "..FF99....",
    ".F99944...",
    "99994444..",
    "994444444.",
    "444444444.",
    "044444440."
});

// Candy: lollipop (10x16), striped candy cane (8x14)
constexpr SpriteData<10, 16> kDecorCandyLollipop = makeSprite<10, 16>({
    "..000000..",
    ".0E7E7E70.",
    "0E7DD7E7E0",
    "07D77D7E70",
    "0ED7E7DE70",
    "07E77DE7E0",
    ".0E7E7E70.",
    "..000000..",
    "....07....",
    "....07....",
    "....07....",
    "....07....",
    "....07....",
    "....07....",
    "....07....",
    "....00...."
});

constexpr SpriteData<8, 14> kDecorCandyCane = makeSprite<8, 14>({
    "..0000..",
    ".07EE70.",
    "07E007E0",
    "0E0..070",
    "00...0E0",
    ".....070",
    ".....0E0",
    ".....070",
    ".....0E0",
    ".....070",
    ".....0E0",
    ".....070",
    ".....0E0",
    ".....00."
});

// White folded paper/powder packet Perk pickup (8x8)
constexpr SpriteData<8, 8> kPerkPacket = makeSprite<8, 8>({
    ".000000.",
    "07777770",
    "06777760",
    "07677670",
    "07766770",
    "07677670",
    "07777770",
    ".000000."
});

constexpr SpriteData<8, 4> kBridgePlank = makeSprite<8, 4>({
    "09999990",
    "04494490",
    "04444440",
    "00000000"
});

// --- UI Icons ---
constexpr SpriteData<7, 7> kCoinSmall = makeSprite<7, 7>({
    ".00000.",
    "0AA7AA0",
    "0A7AA90",
    "0A7AA90",
    "0AA9990",
    ".09990.",
    "..000.."
});

constexpr SpriteData<9, 11> kLock = makeSprite<9, 11>({
    "..00000..",
    ".0666660.",
    ".06...60.",
    ".06...60.",
    "000000000",
    "0AAAAAAA0",
    "0AA000AA0",
    "0AA000AA0",
    "0AAA0AAA0",
    "099909990",
    "000000000"
});

constexpr SpriteData<11, 11> kSpeakerOn = makeSprite<11, 11>({
    ".....00....",
    "....0770.0.",
    "...07770..0",
    "000777700.0",
    "07777770.00",
    "07777770..0",
    "07777770.00",
    "000777700.0",
    "...07770..0",
    "....0770.0.",
    ".....00...."
});

constexpr SpriteData<11, 11> kSpeakerOff = makeSprite<11, 11>({
    "8....00...8",
    ".8..0660.8.",
    "..8066608..",
    "000866680..",
    "06668680...",
    "06666860...",
    "06668680...",
    "000866680..",
    "..8066608..",
    ".8..0660.8.",
    "8....00...8"
});

constexpr SpriteData<10, 10> kPauseIcon = makeSprite<10, 10>({
    ".000..000.",
    "0777007770",
    "0777007770",
    "0777007770",
    "0777007770",
    "0777007770",
    "0777007770",
    "0777007770",
    "0777007770",
    ".000..000."
});

constexpr SpriteData<8, 12> kArrowLeft = makeSprite<8, 12>({
    ".....00.",
    "....0770",
    "...07770",
    "..077770",
    ".0777770",
    "07777770",
    "07777770",
    ".0777770",
    "..077770",
    "...07770",
    "....0770",
    ".....00."
});

constexpr SpriteData<8, 12> kArrowRight = makeSprite<8, 12>({
    ".00.....",
    "0770....",
    "07770...",
    "077770..",
    "0777770.",
    "07777770",
    "07777770",
    "0777770.",
    "077770..",
    "07770...",
    "0770....",
    ".00....."
});

const uint8_t* chickenPixels(game::PlayerAnim anim, uint8_t frame) {
    switch (anim) {
        case game::PlayerAnim::Idle:
            return (frame % 2u == 0u) ? kChickenIdle0.px.data() : kChickenIdle1.px.data();
        case game::PlayerAnim::Walk:
            switch (frame % 4u) {
                case 0: return kChickenWalk0.px.data();
                case 1: return kChickenWalk1.px.data();
                case 2: return kChickenWalk2.px.data();
                default: return kChickenWalk3.px.data();
            }
        case game::PlayerAnim::Jump:
            return kChickenJump.px.data();
        case game::PlayerAnim::Flap:
            return (frame % 2u == 0u) ? kChickenFlap0.px.data() : kChickenFlap1.px.data();
        case game::PlayerAnim::Fall:
            return kChickenFall.px.data();
        default:
            return kChickenIdle0.px.data();
    }
}

} // namespace

void Sprites::drawSprite(hal::Canvas155& canvas,
                         int x, int y,
                         int w, int h,
                         const uint8_t* pixels,
                         int scale) {
    for (int py = 0; py < h; ++py) {
        for (int px = 0; px < w; ++px) {
            const uint8_t c = pixels[py * w + px];
            if (c == K_TRANSPARENT) {
                continue;
            }
            if (scale <= 1) {
                canvas.setPixel(x + px, y + py, c);
            } else {
                canvas.fillRect(x + px * scale, y + py * scale, scale, scale, c);
            }
        }
    }
}

void Sprites::drawSpriteRemap(hal::Canvas155& canvas,
                              int x, int y,
                              int w, int h,
                              const uint8_t* pixels,
                              const RemapTable& remap,
                              int scale) {
    for (int py = 0; py < h; ++py) {
        for (int px = 0; px < w; ++px) {
            const uint8_t raw = pixels[py * w + px];
            if (raw == K_TRANSPARENT) {
                continue;
            }
            const uint8_t c = remap[raw & 0x0Fu];
            if (scale <= 1) {
                canvas.setPixel(x + px, y + py, c);
            } else {
                canvas.fillRect(x + px * scale, y + py * scale, scale, scale, c);
            }
        }
    }
}

void Sprites::drawChicken(hal::Canvas155& canvas,
                          int x, int y,
                          persistence::ChickenId chicken,
                          game::PlayerAnim anim,
                          uint8_t frame,
                          bool locked_silhouette,
                          int scale) {
    const uint8_t* body = chickenPixels(anim, frame);
    if (locked_silhouette) {
        drawSpriteRemap(canvas, x, y, 16, 16, body, kRemapLocked, scale);
        return;
    }

    const RemapTable* remap = &kRemapHenni;
    if (chicken == persistence::ChickenId::Goldie) {
        remap = &kRemapGoldie;
    } else if (chicken == persistence::ChickenId::Ninja) {
        remap = &kRemapNinja;
    }

    drawSpriteRemap(canvas, x, y, 16, 16, body, *remap, scale);

    if (chicken == persistence::ChickenId::Ninja) {
        const uint8_t* hb = ((frame % 2u) == 0u) ? kNinjaHeadband0.px.data()
                                                 : kNinjaHeadband1.px.data();
        drawSprite(canvas, x, y, 16, 16, hb, scale);
    }
}

void Sprites::drawCoin(hal::Canvas155& canvas, int x, int y, uint8_t frame) {
    const uint8_t* px = kCoin0.px.data();
    switch (frame % 4u) {
        case 1: px = kCoin1.px.data(); break;
        case 2: px = kCoin2.px.data(); break;
        case 3: px = kCoin3.px.data(); break;
        default: break;
    }
    drawSprite(canvas, x, y, 8, 8, px, 1);
}

void Sprites::drawCoinSparkle(hal::Canvas155& canvas, int x, int y, uint8_t frame) {
    const uint8_t* px = kSparkle0.px.data();
    if (frame == 1) px = kSparkle1.px.data();
    else if (frame >= 2) px = kSparkle2.px.data();
    drawSprite(canvas, x, y, 8, 8, px, 1);
}

void Sprites::drawPerkPacket(hal::Canvas155& canvas, int x, int y, uint8_t pulse_frame) {
    drawSprite(canvas, x, y, 8, 8, kPerkPacket.px.data(), 1);
    const uint8_t f = static_cast<uint8_t>(pulse_frame % 4u);
    const uint8_t glint = (f == 0) ? C_WHITE : ((f == 1) ? C_LIGHT_BLUE : ((f == 2) ? C_PINK : C_YELLOW));
    canvas.setPixel(x + 6, y + 1, glint);
    if ((f & 1u) == 0u) {
        canvas.setPixel(x + 1, y + 6, C_LIGHT_BLUE);
    }
}

void Sprites::drawObstacle(hal::Canvas155& canvas, int x, int y,
                           world::ObstacleType type, world::BiomeId biome) {
    // Remap table per biome to theme all 7 obstacle shapes while preserving crisp outlines
    static constexpr RemapTable kRemapGreenland{
        C_BLACK, C_DARK_BLUE, C_DARK_PURPLE, C_DARK_GREEN, C_BROWN, C_DARK_GREY, C_LIGHT_GREY, C_WHITE,
        C_RED, C_ORANGE, C_YELLOW, C_GREEN, C_LIGHT_BLUE, C_LAVENDER, C_PINK, C_PEACH};
    static constexpr RemapTable kRemapTaiga{
        C_BLACK, C_DARK_BLUE, C_DARK_PURPLE, C_DARK_GREEN, C_BROWN, C_DARK_GREY, C_LIGHT_GREY, C_DARK_GREEN,
        C_RED, C_BROWN, C_DARK_GREEN, C_DARK_GREEN, C_LIGHT_BLUE, C_LAVENDER, C_PINK, C_PEACH};
    static constexpr RemapTable kRemapSnow{
        C_DARK_BLUE, C_DARK_BLUE, C_DARK_PURPLE, C_LIGHT_BLUE, C_DARK_GREY, C_LIGHT_BLUE, C_WHITE, C_WHITE,
        C_LIGHT_BLUE, C_LIGHT_BLUE, C_WHITE, C_WHITE, C_LIGHT_BLUE, C_LAVENDER, C_PINK, C_PEACH};
    static constexpr RemapTable kRemapLava{
        C_BLACK, C_DARK_BLUE, C_DARK_PURPLE, C_RED, C_DARK_GREY, C_DARK_PURPLE, C_DARK_GREY, C_ORANGE,
        C_RED, C_ORANGE, C_YELLOW, C_DARK_GREY, C_ORANGE, C_LAVENDER, C_PINK, C_PEACH};
    static constexpr RemapTable kRemapJungle{
        C_BLACK, C_DARK_BLUE, C_DARK_PURPLE, C_DARK_GREEN, C_BROWN, C_DARK_GREY, C_DARK_GREEN, C_GREEN,
        C_RED, C_DARK_GREEN, C_GREEN, C_DARK_GREEN, C_LIGHT_BLUE, C_LAVENDER, C_PINK, C_PEACH};
    static constexpr RemapTable kRemapSand{
        C_BLACK, C_DARK_BLUE, C_DARK_PURPLE, C_BROWN, C_BROWN, C_ORANGE, C_PEACH, C_YELLOW,
        C_RED, C_ORANGE, C_YELLOW, C_PEACH, C_LIGHT_BLUE, C_LAVENDER, C_PINK, C_PEACH};
    static constexpr RemapTable kRemapCandy{
        C_DARK_PURPLE, C_DARK_PURPLE, C_DARK_PURPLE, C_PINK, C_LAVENDER, C_PINK, C_WHITE, C_WHITE,
        C_PINK, C_LAVENDER, C_WHITE, C_PINK, C_LIGHT_BLUE, C_LAVENDER, C_PINK, C_WHITE};

    const RemapTable* remap = &kRemapGreenland;
    switch (biome) {
        case world::BiomeId::Taiga:  remap = &kRemapTaiga;  break;
        case world::BiomeId::Snow:   remap = &kRemapSnow;   break;
        case world::BiomeId::Lava:   remap = &kRemapLava;   break;
        case world::BiomeId::Jungle: remap = &kRemapJungle; break;
        case world::BiomeId::Sand:   remap = &kRemapSand;   break;
        case world::BiomeId::Candy:  remap = &kRemapCandy;  break;
        default: break;
    }

    switch (type) {
        case world::ObstacleType::HayBale:
            drawSpriteRemap(canvas, x, y, 8, 8, kHayBale.px.data(), *remap, 1);
            break;
        case world::ObstacleType::Fence:
            drawSpriteRemap(canvas, x, y, 8, 16, kFence.px.data(), *remap, 1);
            break;
        case world::ObstacleType::Stone:
            drawSpriteRemap(canvas, x, y, 16, 16, kStoneBlock.px.data(), *remap, 1);
            break;
        case world::ObstacleType::Tower:
            drawSpriteRemap(canvas, x, y, 8, 24, kTower.px.data(), *remap, 1);
            break;
        case world::ObstacleType::Overhang:
            drawSpriteRemap(canvas, x, y, 24, 12, kOverhang.px.data(), *remap, 1);
            break;
        case world::ObstacleType::WideBarrier:
            drawSpriteRemap(canvas, x, y, 24, 16, kWideBarrier.px.data(), *remap, 1);
            break;
        case world::ObstacleType::Pillar:
            drawSpriteRemap(canvas, x, y, 16, 24, kPillar.px.data(), *remap, 1);
            break;
        default:
            break;
    }
}

void Sprites::drawDecor(hal::Canvas155& canvas, int ground_x, int ground_y,
                        world::DecorKind kind) {
    switch (kind) {
        case world::DecorKind::GreenTree:
            drawSprite(canvas, ground_x, ground_y - 14, 12, 16, kDecorGreenTree.px.data(), 1);
            break;
        case world::DecorKind::GrassTuft:
            drawSprite(canvas, ground_x, ground_y - 4, 8, 4, kDecorGrassTuft.px.data(), 1);
            break;
        case world::DecorKind::Flower:
            drawSprite(canvas, ground_x, ground_y - 6, 6, 6, kDecorFlower.px.data(), 1);
            break;
        case world::DecorKind::Spruce:
            drawSprite(canvas, ground_x, ground_y - 17, 12, 18, kDecorSpruce.px.data(), 1);
            break;
        case world::DecorKind::Birch:
            drawSprite(canvas, ground_x, ground_y - 17, 10, 18, kDecorBirch.px.data(), 1);
            break;
        case world::DecorKind::Rock:
            drawSprite(canvas, ground_x, ground_y - 5, 10, 6, kDecorRock.px.data(), 1);
            break;
        case world::DecorKind::SnowSpruce:
            drawSprite(canvas, ground_x, ground_y - 17, 12, 18, kDecorSnowSpruce.px.data(), 1);
            break;
        case world::DecorKind::LavaSpire:
            drawSprite(canvas, ground_x, ground_y - 13, 10, 14, kDecorLavaSpire.px.data(), 1);
            break;
        case world::DecorKind::LavaCrystal:
            drawSprite(canvas, ground_x, ground_y - 7, 8, 8, kDecorLavaCrystal.px.data(), 1);
            break;
        case world::DecorKind::JungleTree:
            drawSprite(canvas, ground_x, ground_y - 25, 16, 26, kDecorJungleTree.px.data(), 1);
            break;
        case world::DecorKind::JungleTemple:
            drawSprite(canvas, ground_x, ground_y - 15, 22, 16, kDecorJungleTemple.px.data(), 1);
            break;
        case world::DecorKind::Fern:
            drawSprite(canvas, ground_x, ground_y - 5, 8, 5, kDecorFern.px.data(), 1);
            break;
        case world::DecorKind::CactusSmall:
            drawSprite(canvas, ground_x, ground_y - 10, 8, 10, kDecorCactusSmall.px.data(), 1);
            break;
        case world::DecorKind::CactusTall:
            drawSprite(canvas, ground_x, ground_y - 18, 10, 18, kDecorCactusTall.px.data(), 1);
            break;
        case world::DecorKind::DesertRock:
            drawSprite(canvas, ground_x, ground_y - 5, 10, 6, kDecorDesertRock.px.data(), 1);
            break;
        case world::DecorKind::CandyLollipop:
            drawSprite(canvas, ground_x, ground_y - 16, 10, 16, kDecorCandyLollipop.px.data(), 1);
            break;
        case world::DecorKind::CandyCane:
            drawSprite(canvas, ground_x, ground_y - 14, 8, 14, kDecorCandyCane.px.data(), 1);
            break;
        default:
            break;
    }
}

void Sprites::drawBridgePlank(hal::Canvas155& canvas, int x, int y) {
    drawSprite(canvas, x, y, 8, 4, kBridgePlank.px.data(), 1);
}

void Sprites::drawCoinSmall(hal::Canvas155& canvas, int x, int y) {
    drawSprite(canvas, x, y, 7, 7, kCoinSmall.px.data(), 1);
}

void Sprites::drawLock(hal::Canvas155& canvas, int x, int y) {
    drawSprite(canvas, x, y, 9, 11, kLock.px.data(), 1);
}

void Sprites::drawSpeaker(hal::Canvas155& canvas, int x, int y, bool on) {
    drawSprite(canvas, x, y, 11, 11, on ? kSpeakerOn.px.data() : kSpeakerOff.px.data(), 1);
}

void Sprites::drawPauseIcon(hal::Canvas155& canvas, int x, int y, uint8_t color) {
    RemapTable r{0, 1, 2, 3, 4, 5, 6, color, 8, 9, 10, 11, 12, 13, 14, 15};
    drawSpriteRemap(canvas, x, y, 10, 10, kPauseIcon.px.data(), r, 1);
}

void Sprites::drawArrowLeft(hal::Canvas155& canvas, int x, int y, uint8_t color) {
    RemapTable r{0, 1, 2, 3, 4, 5, 6, color, 8, 9, 10, 11, 12, 13, 14, 15};
    drawSpriteRemap(canvas, x, y, 8, 12, kArrowLeft.px.data(), r, 1);
}

void Sprites::drawArrowRight(hal::Canvas155& canvas, int x, int y, uint8_t color) {
    RemapTable r{0, 1, 2, 3, 4, 5, 6, color, 8, 9, 10, 11, 12, 13, 14, 15};
    drawSpriteRemap(canvas, x, y, 8, 12, kArrowRight.px.data(), r, 1);
}

} // namespace schreihuhn::render

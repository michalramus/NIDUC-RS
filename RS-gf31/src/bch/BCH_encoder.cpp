#include "BCH_encoder.hpp"

// Tablice Galois Field
static uint8_t alpha_to[N+1];       // α^i -> wartość
static int8_t index_of[N+1];        // wartość -> i (log)
static uint32_t g_poly = 0;         // wielomian generatora
static uint8_t g_deg = 0;           // stopień g(x)

// Mnożenie w GF(2^m)
static uint8_t gf_mul(uint8_t a, uint8_t b) {
    if (a == 0 || b == 0) return 0;
    int idx = (index_of[a] + index_of[b]) % N;
    return alpha_to[idx];
}

// Inicjalizacja tablicy GF(2^5)
static void gf_init() {
    alpha_to[0] = 1;
    for (int i = 1; i < N; i++) {
        alpha_to[i] = alpha_to[i-1] << 1;
        if (alpha_to[i] & (1 << M))
            alpha_to[i] ^= GPOLY;
    }
    alpha_to[N] = 0;  // α^31 = α^0 = 1 (cykliczne)
    
    for (int i = 0; i < N; i++)
        index_of[alpha_to[i]] = i;
    index_of[0] = -1;  // log(0) = nieokreślone
}

// Oblicz wielomian generatora g(x)
// g(x) = LCM[(m_1(x), m_3(x), ..., m_{2t-1}(x)]
// Dla BCH(31,21,t=2): g(x) = m_1(x) * m_3(x)
static void compute_generator() {
    uint32_t poly = 1;  // zaczynamy od 1
    
    // Dla każdego nieparzystego i od 1 do 2t-1
    for (int root = 1; root <= 2*T - 1; root += 2) {
        // Budujemy wielomian minimalny (x - α^root)
        // Uproszczenie: dla BCH binarnego mnożymy kolejne czynniki
        uint32_t factor = (1 << 1) | alpha_to[root];  // x + α^root
        
        // Mnożenie wielomianów
        uint32_t temp = 0;
        for (int i = 0; i <= g_deg; i++) {
            if (poly & (1 << i)) {
                temp ^= (factor << i);
            }
        }
        poly = temp;
        
        // Aktualizuj stopień
        g_deg = 0;
        uint32_t p = poly;
        while (p) {
            g_deg++;
            p >>= 1;
        }
        g_deg--;
    }
    
    g_poly = poly;
}

// Inicjalizacja kodera BCH
void bch_init() {
    gf_init();
    compute_generator();
}

// Kodowanie: data (21 bitów) -> codeword (31 bitów)
uint32_t bch_encode(uint32_t data) {
    // Maska dla K bitów
    data &= ((1UL << K) - 1);
    
    // Przesunięcie: data * x^(n-k)
    uint32_t reg = data << (N - K);
    
    // Dzielenie wielomianowe (systematyczne kodowanie)
    for (int i = N - 1; i >= N - K; i--) {
        if (reg & (1UL << i)) {
            reg ^= (g_poly << (i - g_deg));
        }
    }
    
    // Reszta to bity parzystości
    uint32_t parity = reg & ((1UL << (N - K)) - 1);
    
    // Słowo kodowe: [data | parity]
    return (data << (N - K)) | parity;
}
#include <random>
#include "SFML/Graphics.hpp"

#define WIDTH 1980
#define HEIGHT 1400
#define CIRCLE_RADIUS 20
#define STEP 15
#define POPULATION_SIZE 150
#define GENES_COUNT 350

const sf::Color BG_COLOR = sf::Color(213, 184, 255);
const sf::Color ITEM_COLOR = sf::Color(255, 255, 255);
const sf::Vector2f  TARGET_POSITION = sf::Vector2f(50, 50);
const sf::Vector2f START_POSITION = sf::Vector2f(1490, 1350);

std::default_random_engine random_engine;
std::uniform_real_distribution<float>range(-2, 2);

struct Individual
{
    sf::Vector2f position;
    std::vector<sf::Vector2f> genes;

    double calc_fitness();
    Individual crossover(Individual partner);
};

double dist(sf::Vector2f p1, sf::Vector2f p2)
{
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

double Individual::calc_fitness() {
    //fixme: that shit working wong
    double distance_to_target = dist(position, TARGET_POSITION);
    double normalised = distance_to_target / HEIGHT;

    return  2.0 - normalised;
}


Individual Individual::crossover(Individual partner)
{
    // partner param represents "daddy" individual
    // Crossover is done in a way: [M,D,M,D,M,D,M,D,M,D,M,D]

    Individual child;
    child.position = START_POSITION;

    for (int i = 0; i < genes.size(); i++)
    {
        if (i % 2 == 0)
            child.genes.push_back(partner.genes[i]);
        else
            child.genes.push_back(genes[i]);
    }

    return child;
}

sf::Vector2f random_vector()
{
    return sf::Vector2f(range(random_engine), range(random_engine));
}

std::vector<sf::Vector2f> create_genes(int n)
{
    std::vector<sf::Vector2f> genes;

    for (int i = 0; i < n; i++)
    {
        auto gene = random_vector();
        genes.push_back(gene);
    }

    return genes;
}

std::vector<Individual> init_population(int n)
{
    std::vector<Individual> population;

    for (int i = 0; i < n; i++)
    {
        Individual individual;
        individual.position = START_POSITION;
        individual.genes = create_genes(GENES_COUNT);

        population.push_back(individual);
    }

    return population;
}

void draw_population(sf::RenderWindow& window, std::vector<Individual>& population, int iteration)
{
    std::vector<Individual> migrated_population;
    for (auto individual : population)
    {
        sf::CircleShape shape = sf::CircleShape(CIRCLE_RADIUS);
        auto direction = individual.genes[iteration];

        shape.setFillColor(ITEM_COLOR);
        shape.setPosition(individual.position);

        window.draw(shape);

        individual.position = sf::Vector2f(individual.position.x + direction.x * STEP, individual.position.y + direction.y * STEP);
        migrated_population.push_back(individual);
    }

    population = migrated_population;
}

void selection(std::vector<Individual>& matingPool, std::vector<Individual>& population)
{
    matingPool.clear();

    for (auto individual : population)
    {
        int prob = floor(individual.calc_fitness() * 100.0);

        for (int i = 0; i < prob; i++)
            matingPool.push_back(individual);
    }
}

void reproduce(std::vector<Individual>& matingPool, std::vector<Individual>& population)
{
    static std::uniform_real_distribution<float> random_range(0, matingPool.size());
    std::vector<Individual> new_population;

    for (int i = 0; i < population.size(); i++)
    {
        int mummy_index = floor(random_range(random_engine));
        int daddy_index = floor(random_range(random_engine));

        auto mummy = matingPool[mummy_index];
        auto daddy = matingPool[daddy_index];

        auto child = mummy.crossover(daddy);
        //TODO: add mutation
        new_population.push_back(child);
    }

    population = new_population;
}

int main() {
    //TODO: rethink how u treat coordinates, cause that shit is wrong and inconvenient
    //TODO: add text info to the screen

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Genetic algorithm");
    window.setFramerateLimit(60);

    sf::RectangleShape target = sf::RectangleShape(TARGET_POSITION);
    target.setFillColor(ITEM_COLOR);
    target.setPosition(sf::Vector2f(990, 10));

    std::vector<Individual> population = init_population(POPULATION_SIZE);
    std::vector<Individual> matingPool;

    int genes_counter = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(BG_COLOR);
        window.draw(target);

        if (genes_counter < GENES_COUNT)
        {
            draw_population(window, population, genes_counter);
            genes_counter++;
        }

        if (genes_counter == GENES_COUNT)
        {
            selection(matingPool, population);
            reproduce(matingPool, population);
            genes_counter = 0;
        }

        window.display();
    }

    return 0;
}

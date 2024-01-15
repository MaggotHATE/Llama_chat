#pragma once

namespace llmcpp {
    struct stats {
        int strength = 5;
        int intelligence = 5;
        int agility = 5;
        int charisma = 5;
        int luck = 5;
    };
    
    struct action_type {
        std::string action_descr = "move";
        std::string action_descr_s = "moves";
        std::string action_descr_ing = "moving";
        char primary = 's';
        char secondary = 'a';
    };
    
    class Object {
        private:
            stats object_stats;
            
            char type = 'o';
        public:
            int get(char property) {
                switch (property) {
                    case 's': return object_stats.strength;
                    case 'i': return object_stats.intelligence;
                    case 'a': return object_stats.agility;
                    case 'c': return object_stats.charisma;
                    case 'l': return object_stats.luck;
                    default: return type;
                }
            }
    };
    
    class Human : public Object {
        private:
            stats permastats {5,6,4,7,7};
            stats mood {1,1,1,1,1};
            std::string name = "Jack";
            std::string title = "knight";
            char type = 'h'; // living creature
        public:
            int get(char property) {
                switch (property) {
                    case 's': return permastats.strength;
                    case 'i': return permastats.intelligence;
                    case 'a': return permastats.agility;
                    case 'c': return permastats.charisma;
                    case 'l': return permastats.luck;
                    default: return type;
                }
            }
            
            std::string get_name() {
                return name;
            }
            
            std::string get_title() {
                return title;
            }
        
            Human(int s, int i, int a, int c, int l, std::string new_name = "Jack", std::string new_title = "knight") {
                permastats.strength = s;
                permastats.intelligence = i;
                permastats.agility = a;
                permastats.charisma = c;
                permastats.luck = l;
                name = new_name;
                title = new_title;
            }
            
            void act(int action_result, char action_mood) {
                switch (action_mood) {
                    case 's': mood.strength += 1 - action_result;
                    case 'i': mood.intelligence += 1 - action_result;
                    case 'a': mood.agility += 1 - action_result;
                    case 'c': mood.charisma += 1 - action_result;
                    case 'l': mood.luck += 1 - action_result;
                    default: {
                        mood.strength += 1 - action_result;
                        mood.intelligence += 1 - action_result;
                        mood.agility += 1 - action_result;
                        mood.charisma += 1 - action_result;
                        mood.luck += 1 - action_result;
                    }
                }
            }
    };
    
    class Item : public Object {
        private:
            // int strength = 7; // sturdy
            // int intelligence = 1; // uninteresting
            // int agility = 1; // hard to move around
            // int charisma = 1; // unintimidating
            // int luck = 0; // no luck bonus
            stats object_stats {7,1,1,1,0};
            std::string name = "table";
            
            char type = 'o'; // object
        public:
            int get(char property) {
                switch (property) {
                    case 's': return object_stats.strength;
                    case 'i': return object_stats.intelligence;
                    case 'a': return object_stats.agility;
                    case 'c': return object_stats.charisma;
                    case 'l': return object_stats.luck;
                    default: return type;
                }
            }
            
            std::string get_name() {
                return name;
            }
        
            Item(int s, int i, int a, int c, int l, std::string new_name = "table") {
                object_stats.strength = s;
                object_stats.intelligence = i;
                object_stats.agility = a;
                object_stats.charisma = c;
                object_stats.luck = l;
                
                name = new_name;
            }
            
            char get_important() {
                int max_stat = std::max({object_stats.strength, object_stats.intelligence, object_stats.agility, object_stats.charisma, object_stats.luck});
                // switch (max_stat) {
                    // case object_stats.strength: return 's';
                    // case object_stats.intelligence: return 'i';
                    // case object_stats.agility: return 'a';
                    // case object_stats.charisma: return 'c';
                    // case object_stats.luck: return 'l';
                    // default: return 'l';
                // }
                if (max_stat == object_stats.strength) return 's';
                else if (max_stat == object_stats.intelligence) return 'i';
                else if (max_stat == object_stats.agility) return 'a';
                else if (max_stat == object_stats.charisma) return 'c';
                else return 'l';
            }
    };
    
    class Place : public Object {
        private:
            // int strength = 7; // luxurious|wasted
            // int intelligence = 1; // furnished|empty
            // int agility = 1; // small|spacious
            // int charisma = 1; // cozy|depressing
            // int luck = 0; // no luck bonus
            stats place_stats {5,2,5,7,1};
            std::string name = "room";
            
            
            char type = 'p'; // place
        public:
            std::map<std::string,Item&> items;
            std::map<std::string,Human&> characters;
        
            int get(char property) {
                switch (property) {
                    case 's': return place_stats.strength;
                    case 'i': return place_stats.intelligence;
                    case 'a': return place_stats.agility;
                    case 'c': return place_stats.charisma;
                    case 'l': return place_stats.luck;
                    default: return type;
                }
            }
            
            std::string get_name() {
                return name;
            }
        
            Place(int s, int i, int a, int c, int l, std::string new_name = "room") {
                place_stats.strength = s;
                place_stats.intelligence = i;
                place_stats.agility = a;
                place_stats.charisma = c;
                place_stats.luck = l;
                
                name = new_name;
            }
            
            void place(Item& placeable_item) {
                items.insert({placeable_item.get_name(),placeable_item});
            }
            
            void inhabit(Human& character) {
                characters.insert({character.get_name(),character});
            }
            
            char get_important() {
                int max_stat = std::max({place_stats.strength, place_stats.intelligence, place_stats.agility, place_stats.charisma, place_stats.luck});

                if (max_stat == place_stats.strength) return 's';
                else if (max_stat == place_stats.intelligence) return 'i';
                else if (max_stat == place_stats.agility) return 'a';
                else if (max_stat == place_stats.charisma) return 'c';
                else return 'l';
            }
    };
    
    static int move(Human& character, Item& subject) {
        if (character.get('t') == 'o') {
            return 0; // inanimate objects cannot move other objects themselves
        }
        int mover = (character.get('s') + character.get('a') + character.get('l') - 4);
        int moved = ( subject.get('s') - subject.get('a') - subject.get('l'));
        
        return mover - moved;
        // std::cout << std::to_string(mover) << " vs " << std::to_string(moved) << std::endl;
        
        // if ( mover > moved ) {
            // return true;
        // } else return false;
    }
    
    static int act(Human& character, Item& subject, char primary, char secondary) {
        if (character.get('t') == 'o') {
            return 0; // inanimate objects cannot move other objects themselves
        }
        int actor = (character.get(primary) + character.get(secondary) + character.get('l') - 4);
        int acted = ( subject.get(primary) - subject.get(secondary) - subject.get('l'));
        
        return actor - acted;
    }
    
    
    static std::string get_mood_estimate(int val, std::string verb) {
        std::string description;
        int diff = val - 1;
        
        if (diff > 5) description = "needs to " + verb;
        else if (diff > 3) description = "wants to " + verb;
        else if (diff < 0) description = "hates to " + verb;
        else if (diff == 0) description = "doesn't want to " + verb;
        
        return description;
    }
    
    static std::string get_stat_estimate(int val, int default_val, std::string adjective_pos, std::string adjective_neg) {
        std::string description;
        int diff = val - default_val;
        
        if (diff > 2) description = "very " + adjective_pos;
        else if (diff > 1) description = adjective_pos;
        else if (diff < -2) description = "very " + adjective_neg;
        else if (diff < -1) description = adjective_neg;
        else description = "";
        
        return description;
    }
    
    static std::string get_description(Human& character) {
        std::string description = " ";
        
        description += get_stat_estimate(character.get('s'), 5, "strong", "weak");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(character.get('i'), 5, "smart", "dumb");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(character.get('a'), 5, "agile", "clumsy");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(character.get('c'), 5, "charismatic", "awkward");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(character.get('l'), 5, "lucky", "unlucky");
        description += ".";
        
        return description;
    }
    
    static std::string get_description(Item& subject) {
        //std::string description = subject.get_name() + " is ";
        std::string description = " ";
        
        description += get_stat_estimate(subject.get('s'), 5, "heavy", "light");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(subject.get('i'), 5, "interesting", "simple");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(subject.get('a'), 5, "handy", "bulky");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(subject.get('c'), 5, "peculiar", "plain");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(subject.get('l'), 5, "special", "typical");
        description += ".";
        
        return description;
    }
    
    static std::string get_description_important(Human& character) {
        std::string description = character.get_name() + " the " + character.get_title() + " is ";
        int max_stat = std::max({character.get('s'), character.get('i'), character.get('a'), character.get('c'), character.get('l')});

        if (max_stat == character.get('s')) return get_stat_estimate(character.get('s'), 5, "strong", "weak");
        else if (max_stat == character.get('i')) return get_stat_estimate(character.get('i'), 5, "smart", "dumb");
        else if (max_stat == character.get('a')) return get_stat_estimate(character.get('a'), 5, "agile", "clumsy");
        else if (max_stat == character.get('c')) return get_stat_estimate(character.get('c'), 5, "charismatic", "awkward");
        else if (max_stat == character.get('l')) return get_stat_estimate(character.get('l'), 5, "lucky", "unlucky");
        else return "average";
        
    }
    
    static std::string get_description_important(Item& subject) {
        //std::string description = subject.get_name() + " is ";
        std::string description = " ";
        int max_stat = std::max({subject.get('s'), subject.get('i'), subject.get('a'), subject.get('c'), subject.get('l')});

        if (max_stat == subject.get('s')) return get_stat_estimate(subject.get('s'), 5, "heavy", "light");
        else if (max_stat == subject.get('i')) return get_stat_estimate(subject.get('i'), 5, "interesting", "simple");
        else if (max_stat == subject.get('a')) return get_stat_estimate(subject.get('a'), 5, "handy", "bulky");
        else if (max_stat == subject.get('c')) return get_stat_estimate(subject.get('c'), 5, "peculiar", "plain");
        else if (max_stat == subject.get('l')) return get_stat_estimate(subject.get('l'), 5, "special", "typical");
        else return "average";

    }
    
    static std::string get_description(Place& location) {
        //std::string description = location.get_name() + " is ";
        std::string description = " ";
        
        description += get_stat_estimate(location.get('s'), 5, "luxurious", "desolated");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(location.get('i'), 5, "furnished", "empty");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(location.get('a'), 5, "small", "spaceous");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(location.get('c'), 5, "cozy", "horrible");
        if (description.back() != ',' && description.back() != ' ') description += ", ";
        description += get_stat_estimate(location.get('l'), 5, "special", "common");
        description += ".";
        
        return description;
    }
    
    static std::string get_items_description(Place& location) {
        std::string description = "there's";
        for (auto i : location.items) {
            description += get_description_important(i.second) + " " + i.first;
            description += ", ";
        }
        
        description.pop_back();
        description.pop_back();
        
        return description;
    }
    
    static std::string get_characters_description(Place& location) {
        std::string description = " ";
        for (auto i : location.characters) {
            description += get_description_important(i.second) + " " + i.first;
            description += ", ";
        }
        
        description.pop_back();
        description.pop_back();
        
        return description;
    }
    
    static std::string move_action(Human& character, Item& subject) {
        char stat = subject.get_important();
        int result = move(character, subject);
        std::string action_name = "moving the " + subject.get_name();
        
        std::string character_names = character.get_name() + " the " + character.get_title();
        
        std::string result_string = character_names + " " + get_mood_estimate(character.get(stat),action_name) + " and ";
        character.act(result, stat);
        if (result > 1 ) return result_string + ("moves " + subject.get_name());
        else return (result_string + "tried, but couldn't move " + subject.get_name());
        
    }
    
    static std::string action_mood(Human& character, Item& subject, action_type action) {
        char stat = subject.get_important();
        //std::string character_names = character.get_name() + " the " + character.get_title();
        std::string action_name = action.action_descr + " the " + subject.get_name();
        
        return (get_mood_estimate(character.get(stat),action_name) );
    }
    
    static std::string action_make(Human& character, Item& subject, action_type action) {
        char stat = subject.get_important();
        int result = act(character, subject, action.primary, action.secondary);
        
        std::string character_names = character.get_name() + " the " + character.get_title();

        character.act(result, stat);
        if (result > 1 ) return character_names + " " + (action.action_descr_s + " " + subject.get_name());
        else return (character_names + " tried, but couldn't" + action.action_descr + " " + subject.get_name());
    }
    
};
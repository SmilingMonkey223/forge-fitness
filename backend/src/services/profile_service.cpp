#include "../../include/profile_service.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <regex>

namespace forge {

// --- Validation helpers ---

void ProfileService::validate_sex(const std::string& sex) {
    if (sex != "male" && sex != "female" && sex != "other") {
        throw std::invalid_argument("INVALID_SEX");
    }
}

void ProfileService::validate_activity_level(const std::string& activity_level) {
    static const std::vector<std::string> valid = {
        "sedentary", "lightly_active", "moderately_active",
        "very_active", "extremely_active"
    };
    for (const auto& v : valid) {
        if (activity_level == v) return;
    }
    throw std::invalid_argument("INVALID_ACTIVITY_LEVEL");
}

void ProfileService::validate_fitness_goal(const std::string& fitness_goal) {
    if (fitness_goal != "lose_fat" && fitness_goal != "maintain" && fitness_goal != "build_muscle") {
        throw std::invalid_argument("INVALID_FITNESS_GOAL");
    }
}

void ProfileService::validate_date_of_birth(const std::string& dob) {
    // Validate format YYYY-MM-DD
    static const std::regex pattern(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!std::regex_match(dob, pattern)) {
        throw std::invalid_argument("INVALID_DATE_OF_BIRTH");
    }

    // Parse and validate the date components
    int year, month, day;
    char dash1, dash2;
    std::istringstream iss(dob);
    iss >> year >> dash1 >> month >> dash2 >> day;

    if (month < 1 || month > 12 || day < 1) {
        throw std::invalid_argument("INVALID_DATE_OF_BIRTH");
    }

    // Per-month day validation
    static const int days_in_month[] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (day > days_in_month[month]) {
        throw std::invalid_argument("INVALID_DATE_OF_BIRTH");
    }
    // Reject Feb 29 on non-leap years
    if (month == 2 && day == 29) {
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (!leap) {
            throw std::invalid_argument("INVALID_DATE_OF_BIRTH");
        }
    }

    // Check age is reasonable (13-120 years)
    int age = calculate_age(dob);
    if (age < 13 || age > 120) {
        throw std::invalid_argument("INVALID_AGE");
    }
}

void ProfileService::validate_height(double height_cm) {
    if (height_cm < 100.0 || height_cm > 250.0) {
        throw std::invalid_argument("INVALID_HEIGHT");
    }
}

void ProfileService::validate_weight(double weight_kg) {
    if (weight_kg < 30.0 || weight_kg > 300.0) {
        throw std::invalid_argument("INVALID_WEIGHT");
    }
}

// --- Calculation methods ---

int ProfileService::calculate_age(const std::string& date_of_birth) {
    int birth_year, birth_month, birth_day;
    char dash1, dash2;
    std::istringstream iss(date_of_birth);
    iss >> birth_year >> dash1 >> birth_month >> dash2 >> birth_day;

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf = {};
    gmtime_r(&time_t_now, &tm_buf);

    int current_year = tm_buf.tm_year + 1900;
    int current_month = tm_buf.tm_mon + 1;
    int current_day = tm_buf.tm_mday;

    int age = current_year - birth_year;

    // Adjust if birthday hasn't occurred yet this year
    if (current_month < birth_month ||
        (current_month == birth_month && current_day < birth_day)) {
        age--;
    }

    return age;
}

double ProfileService::calculate_bmr(const std::string& sex, double weight_kg,
                                      double height_cm, int age) {
    // Mifflin-St Jeor equation
    // Male:   BMR = 10 * weight(kg) + 6.25 * height(cm) - 5 * age - 161 + 166
    //       = 10 * weight(kg) + 6.25 * height(cm) - 5 * age + 5
    // Female: BMR = 10 * weight(kg) + 6.25 * height(cm) - 5 * age - 161
    double bmr = 10.0 * weight_kg + 6.25 * height_cm - 5.0 * age - 161.0;

    if (sex == "male") {
        bmr += 166.0;  // -161 + 166 = +5 (standard Mifflin-St Jeor male offset)
    }
    // For "other", use the average of male and female
    if (sex == "other") {
        bmr += 83.0;  // midpoint between 0 and 166
    }

    return bmr;
}

double ProfileService::get_activity_multiplier(const std::string& activity_level) {
    if (activity_level == "sedentary") return 1.2;
    if (activity_level == "lightly_active") return 1.375;
    if (activity_level == "moderately_active") return 1.55;
    if (activity_level == "very_active") return 1.725;
    if (activity_level == "extremely_active") return 1.9;
    throw std::invalid_argument("INVALID_ACTIVITY_LEVEL");
}

double ProfileService::calculate_tdee(const std::string& sex, double weight_kg,
                                       double height_cm, int age,
                                       const std::string& activity_level) {
    double bmr = calculate_bmr(sex, weight_kg, height_cm, age);
    double multiplier = get_activity_multiplier(activity_level);
    return bmr * multiplier;
}

ProfileService::MacroTargets ProfileService::calculate_macros(
    const std::string& sex, double weight_kg, double height_cm,
    int age, const std::string& activity_level,
    const std::string& fitness_goal) {

    double tdee = calculate_tdee(sex, weight_kg, height_cm, age, activity_level);

    MacroTargets targets;
    targets.tdee_calories = std::round(tdee);

    double protein_per_kg = 1.8;
    double fat_per_kg = 1.0;

    if (fitness_goal == "lose_fat") {
        targets.target_calories = std::round(tdee - 500.0);
        protein_per_kg = 2.0;
        fat_per_kg = 0.9;
    } else if (fitness_goal == "maintain") {
        targets.target_calories = std::round(tdee);
        protein_per_kg = 1.8;
        fat_per_kg = 1.0;
    } else if (fitness_goal == "build_muscle") {
        targets.target_calories = std::round(tdee + 300.0);
        protein_per_kg = 2.2;
        fat_per_kg = 1.0;
    } else {
        throw std::invalid_argument("INVALID_FITNESS_GOAL");
    }

    targets.target_protein_g = std::round(protein_per_kg * weight_kg);
    targets.target_fat_g = std::round(fat_per_kg * weight_kg);

    // Remaining calories from carbs
    // Protein: 4 cal/g, Carbs: 4 cal/g, Fat: 9 cal/g
    double protein_calories = targets.target_protein_g * 4.0;
    double fat_calories = targets.target_fat_g * 9.0;
    double remaining_calories = targets.target_calories - protein_calories - fat_calories;
    targets.target_carbs_g = std::round(std::max(0.0, remaining_calories / 4.0));

    return targets;
}

// --- Database operations ---

std::optional<UserProfile> ProfileService::get_profile(const std::string& user_id) {
    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT id, user_id, date_of_birth, sex, height_cm, weight_kg, "
        "activity_level, fitness_goal, unit_preference, "
        "tdee_calories, target_calories, target_protein_g, target_carbs_g, target_fat_g "
        "FROM user_profiles WHERE user_id = $1",
        user_id
    );

    txn.commit();

    if (result.empty()) {
        return std::nullopt;
    }

    auto row = result[0];
    UserProfile profile;
    profile.id = row["id"].c_str();
    profile.user_id = row["user_id"].c_str();
    profile.date_of_birth = row["date_of_birth"].c_str();
    profile.sex = row["sex"].c_str();
    profile.height_cm = row["height_cm"].as<double>();
    profile.weight_kg = row["weight_kg"].as<double>();
    profile.activity_level = row["activity_level"].c_str();
    profile.fitness_goal = row["fitness_goal"].c_str();
    profile.unit_preference = row["unit_preference"].c_str();

    if (!row["tdee_calories"].is_null())
        profile.tdee_calories = row["tdee_calories"].as<double>();
    if (!row["target_calories"].is_null())
        profile.target_calories = row["target_calories"].as<double>();
    if (!row["target_protein_g"].is_null())
        profile.target_protein_g = row["target_protein_g"].as<double>();
    if (!row["target_carbs_g"].is_null())
        profile.target_carbs_g = row["target_carbs_g"].as<double>();
    if (!row["target_fat_g"].is_null())
        profile.target_fat_g = row["target_fat_g"].as<double>();

    return profile;
}

UserProfile ProfileService::complete_onboarding(const OnboardingRequest& req) {
    // Validate all fields
    validate_sex(req.sex);
    validate_activity_level(req.activity_level);
    validate_fitness_goal(req.fitness_goal);
    validate_date_of_birth(req.date_of_birth);
    validate_height(req.height_cm);
    validate_weight(req.weight_kg);
    if (req.unit_preference != "metric" && req.unit_preference != "imperial") {
        throw std::invalid_argument("INVALID_UNIT_PREFERENCE");
    }

    // Check if profile already exists
    auto existing = get_profile(req.user_id);
    if (existing.has_value()) {
        throw std::invalid_argument("PROFILE_ALREADY_EXISTS");
    }

    // Calculate macros
    int age = calculate_age(req.date_of_birth);
    auto macros = calculate_macros(req.sex, req.weight_kg, req.height_cm,
                                    age, req.activity_level, req.fitness_goal);

    // Insert profile
    std::string profile_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    txn.exec_params(
        "INSERT INTO user_profiles "
        "(id, user_id, date_of_birth, sex, height_cm, weight_kg, "
        "activity_level, fitness_goal, unit_preference, "
        "tdee_calories, target_calories, target_protein_g, target_carbs_g, target_fat_g) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14)",
        profile_id, req.user_id, req.date_of_birth, req.sex,
        req.height_cm, req.weight_kg,
        req.activity_level, req.fitness_goal, req.unit_preference,
        macros.tdee_calories, macros.target_calories,
        macros.target_protein_g, macros.target_carbs_g, macros.target_fat_g
    );

    txn.commit();

    // Build and return the profile
    UserProfile profile;
    profile.id = profile_id;
    profile.user_id = req.user_id;
    profile.date_of_birth = req.date_of_birth;
    profile.sex = req.sex;
    profile.height_cm = req.height_cm;
    profile.weight_kg = req.weight_kg;
    profile.activity_level = req.activity_level;
    profile.fitness_goal = req.fitness_goal;
    profile.unit_preference = req.unit_preference;
    profile.tdee_calories = macros.tdee_calories;
    profile.target_calories = macros.target_calories;
    profile.target_protein_g = macros.target_protein_g;
    profile.target_carbs_g = macros.target_carbs_g;
    profile.target_fat_g = macros.target_fat_g;

    return profile;
}

UserProfile ProfileService::update_profile(const UpdateProfileRequest& req) {
    // Get existing profile
    auto existing = get_profile(req.user_id);
    if (!existing.has_value()) {
        throw std::invalid_argument("PROFILE_NOT_FOUND");
    }

    auto profile = existing.value();

    // Apply updates with validation
    if (req.date_of_birth.has_value()) {
        validate_date_of_birth(*req.date_of_birth);
        profile.date_of_birth = *req.date_of_birth;
    }
    if (req.sex.has_value()) {
        validate_sex(*req.sex);
        profile.sex = *req.sex;
    }
    if (req.height_cm.has_value()) {
        validate_height(*req.height_cm);
        profile.height_cm = *req.height_cm;
    }
    if (req.weight_kg.has_value()) {
        validate_weight(*req.weight_kg);
        profile.weight_kg = *req.weight_kg;
    }
    if (req.activity_level.has_value()) {
        validate_activity_level(*req.activity_level);
        profile.activity_level = *req.activity_level;
    }
    if (req.fitness_goal.has_value()) {
        validate_fitness_goal(*req.fitness_goal);
        profile.fitness_goal = *req.fitness_goal;
    }
    if (req.unit_preference.has_value()) {
        if (*req.unit_preference != "metric" && *req.unit_preference != "imperial") {
            throw std::invalid_argument("INVALID_UNIT_PREFERENCE");
        }
        profile.unit_preference = *req.unit_preference;
    }

    // Recalculate TDEE and macros
    int age = calculate_age(profile.date_of_birth);
    auto macros = calculate_macros(profile.sex, profile.weight_kg, profile.height_cm,
                                    age, profile.activity_level, profile.fitness_goal);

    profile.tdee_calories = macros.tdee_calories;
    profile.target_calories = macros.target_calories;
    profile.target_protein_g = macros.target_protein_g;
    profile.target_carbs_g = macros.target_carbs_g;
    profile.target_fat_g = macros.target_fat_g;

    // Update in database
    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    txn.exec_params(
        "UPDATE user_profiles SET "
        "date_of_birth = $1, sex = $2, height_cm = $3, weight_kg = $4, "
        "activity_level = $5, fitness_goal = $6, unit_preference = $7, "
        "tdee_calories = $8, target_calories = $9, "
        "target_protein_g = $10, target_carbs_g = $11, target_fat_g = $12 "
        "WHERE user_id = $13",
        profile.date_of_birth, profile.sex, profile.height_cm, profile.weight_kg,
        profile.activity_level, profile.fitness_goal, profile.unit_preference,
        macros.tdee_calories, macros.target_calories,
        macros.target_protein_g, macros.target_carbs_g, macros.target_fat_g,
        req.user_id
    );

    txn.commit();

    return profile;
}

} // namespace forge

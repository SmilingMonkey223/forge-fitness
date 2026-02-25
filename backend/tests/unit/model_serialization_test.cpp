#include <gtest/gtest.h>
#include "models.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <chrono>

using json = nlohmann::json;

// ── User::to_json() ─────────────────────────────────────────────────

TEST(UserSerializationUnit, ContainsRequiredFields) {
    forge::User user;
    user.id = "user-123";
    user.email = "test@example.com";
    user.username = "testuser";
    user.display_name = "Test User";
    user.password_hash = "secret_hash_value";

    auto j = user.to_json();

    EXPECT_EQ(j["id"], "user-123");
    EXPECT_EQ(j["email"], "test@example.com");
    EXPECT_EQ(j["username"], "testuser");
    EXPECT_EQ(j["display_name"], "Test User");
}

TEST(UserSerializationUnit, DoesNotIncludePasswordHash) {
    forge::User user;
    user.id = "user-123";
    user.email = "test@example.com";
    user.username = "testuser";
    user.display_name = "Test User";
    user.password_hash = "super_secret_hash";

    auto j = user.to_json();

    EXPECT_FALSE(j.contains("password_hash"))
        << "User JSON must NOT expose password_hash for security";
    EXPECT_FALSE(j.contains("password"))
        << "User JSON must NOT expose password for security";
}

TEST(UserSerializationUnit, DoesNotIncludeTimestamps) {
    forge::User user;
    user.id = "user-123";
    user.email = "test@example.com";
    user.username = "testuser";
    user.display_name = "Test User";

    auto j = user.to_json();

    EXPECT_FALSE(j.contains("created_at"));
    EXPECT_FALSE(j.contains("updated_at"));
    EXPECT_FALSE(j.contains("deleted_at"));
}

// ── UserProfile::to_json() ──────────────────────────────────────────

TEST(UserProfileSerializationUnit, ContainsAllBaseFields) {
    forge::UserProfile profile;
    profile.id = "profile-1";
    profile.user_id = "user-1";
    profile.date_of_birth = "1995-06-15";
    profile.sex = "male";
    profile.height_cm = 180.0;
    profile.weight_kg = 80.0;
    profile.activity_level = "moderately_active";
    profile.fitness_goal = "build_muscle";
    profile.unit_preference = "metric";

    auto j = profile.to_json();

    EXPECT_EQ(j["id"], "profile-1");
    EXPECT_EQ(j["user_id"], "user-1");
    EXPECT_EQ(j["date_of_birth"], "1995-06-15");
    EXPECT_EQ(j["sex"], "male");
    EXPECT_DOUBLE_EQ(j["height_cm"].get<double>(), 180.0);
    EXPECT_DOUBLE_EQ(j["weight_kg"].get<double>(), 80.0);
    EXPECT_EQ(j["activity_level"], "moderately_active");
    EXPECT_EQ(j["fitness_goal"], "build_muscle");
    EXPECT_EQ(j["unit_preference"], "metric");
}

TEST(UserProfileSerializationUnit, OptionalFieldsAbsentWhenUnset) {
    forge::UserProfile profile;
    profile.id = "profile-1";
    profile.user_id = "user-1";

    auto j = profile.to_json();

    EXPECT_FALSE(j.contains("tdee_calories"));
    EXPECT_FALSE(j.contains("target_calories"));
    EXPECT_FALSE(j.contains("target_protein_g"));
    EXPECT_FALSE(j.contains("target_carbs_g"));
    EXPECT_FALSE(j.contains("target_fat_g"));
}

TEST(UserProfileSerializationUnit, OptionalFieldsPresentWhenSet) {
    forge::UserProfile profile;
    profile.id = "profile-1";
    profile.user_id = "user-1";
    profile.tdee_calories = 2500.0;
    profile.target_calories = 2200.0;
    profile.target_protein_g = 176.0;
    profile.target_carbs_g = 220.0;
    profile.target_fat_g = 73.0;

    auto j = profile.to_json();

    EXPECT_DOUBLE_EQ(j["tdee_calories"].get<double>(), 2500.0);
    EXPECT_DOUBLE_EQ(j["target_calories"].get<double>(), 2200.0);
    EXPECT_DOUBLE_EQ(j["target_protein_g"].get<double>(), 176.0);
    EXPECT_DOUBLE_EQ(j["target_carbs_g"].get<double>(), 220.0);
    EXPECT_DOUBLE_EQ(j["target_fat_g"].get<double>(), 73.0);
}

// ── Exercise::to_json() ─────────────────────────────────────────────

TEST(ExerciseSerializationUnit, ContainsAllFields) {
    forge::Exercise ex;
    ex.id = "ex-1";
    ex.name = "Bench Press";
    ex.muscle_group = "chest";
    ex.equipment = "barbell";
    ex.is_custom = false;

    auto j = ex.to_json();

    EXPECT_EQ(j["id"], "ex-1");
    EXPECT_EQ(j["name"], "Bench Press");
    EXPECT_EQ(j["muscle_group"], "chest");
    EXPECT_EQ(j["equipment"], "barbell");
    EXPECT_FALSE(j["is_custom"].get<bool>());
}

TEST(ExerciseSerializationUnit, WithCreatedBy) {
    forge::Exercise ex;
    ex.id = "ex-custom-1";
    ex.name = "My Exercise";
    ex.muscle_group = "legs";
    ex.equipment = "bodyweight";
    ex.is_custom = true;
    ex.created_by = "user-123";

    auto j = ex.to_json();

    EXPECT_TRUE(j.contains("created_by"));
    EXPECT_EQ(j["created_by"], "user-123");
    EXPECT_TRUE(j["is_custom"].get<bool>());
}

TEST(ExerciseSerializationUnit, WithoutCreatedBy) {
    forge::Exercise ex;
    ex.id = "ex-1";
    ex.name = "Squat";
    ex.muscle_group = "legs";
    ex.equipment = "barbell";
    ex.is_custom = false;

    auto j = ex.to_json();

    EXPECT_FALSE(j.contains("created_by"));
}

// ── ExerciseSet::to_json() ──────────────────────────────────────────

TEST(ExerciseSetSerializationUnit, ContainsRequiredFields) {
    forge::ExerciseSet set;
    set.id = "set-1";
    set.workout_id = "w-1";
    set.exercise_id = "ex-1";
    set.set_order = 1;
    set.exercise_order = 1;
    set.set_type = "working";
    set.is_pr = false;

    auto j = set.to_json();

    EXPECT_EQ(j["id"], "set-1");
    EXPECT_EQ(j["workout_id"], "w-1");
    EXPECT_EQ(j["exercise_id"], "ex-1");
    EXPECT_EQ(j["set_order"], 1);
    EXPECT_EQ(j["exercise_order"], 1);
    EXPECT_EQ(j["set_type"], "working");
    EXPECT_FALSE(j["is_pr"].get<bool>());
}

TEST(ExerciseSetSerializationUnit, WithAllOptionalFields) {
    forge::ExerciseSet set;
    set.id = "set-1";
    set.workout_id = "w-1";
    set.exercise_id = "ex-1";
    set.set_order = 1;
    set.exercise_order = 1;
    set.set_type = "working";
    set.is_pr = true;
    set.reps = 8;
    set.weight_kg = 100.0;
    set.duration_seconds = 45;
    set.rpe = 8.5;
    set.rest_seconds = 90;
    set.notes = "Felt strong";

    auto j = set.to_json();

    EXPECT_EQ(j["reps"], 8);
    EXPECT_DOUBLE_EQ(j["weight_kg"].get<double>(), 100.0);
    EXPECT_EQ(j["duration_seconds"], 45);
    EXPECT_DOUBLE_EQ(j["rpe"].get<double>(), 8.5);
    EXPECT_EQ(j["rest_seconds"], 90);
    EXPECT_EQ(j["notes"], "Felt strong");
    EXPECT_TRUE(j["is_pr"].get<bool>());
}

TEST(ExerciseSetSerializationUnit, OptionalFieldsAbsentWhenUnset) {
    forge::ExerciseSet set;
    set.id = "set-1";
    set.workout_id = "w-1";
    set.exercise_id = "ex-1";
    set.set_order = 1;
    set.exercise_order = 1;
    set.set_type = "warmup";
    set.is_pr = false;

    auto j = set.to_json();

    EXPECT_FALSE(j.contains("reps"));
    EXPECT_FALSE(j.contains("weight_kg"));
    EXPECT_FALSE(j.contains("duration_seconds"));
    EXPECT_FALSE(j.contains("rpe"));
    EXPECT_FALSE(j.contains("rest_seconds"));
    EXPECT_FALSE(j.contains("notes"));
}

// ── Workout::to_json() ─────────────────────────────────────────────

TEST(WorkoutSerializationUnit, ContainsRequiredFields) {
    forge::Workout workout;
    workout.id = "w-1";
    workout.user_id = "user-1";
    workout.status = "in_progress";
    workout.started_at = std::chrono::system_clock::now();

    auto j = workout.to_json();

    EXPECT_EQ(j["id"], "w-1");
    EXPECT_EQ(j["user_id"], "user-1");
    EXPECT_EQ(j["status"], "in_progress");
    EXPECT_TRUE(j.contains("started_at"));
    EXPECT_TRUE(j.contains("sets"));
    EXPECT_TRUE(j["sets"].is_array());
}

TEST(WorkoutSerializationUnit, WithOptionalFields) {
    forge::Workout workout;
    workout.id = "w-1";
    workout.user_id = "user-1";
    workout.status = "completed";
    workout.started_at = std::chrono::system_clock::now();
    workout.completed_at = std::chrono::system_clock::now();
    workout.name = "Chest Day";
    workout.duration_seconds = 3600;
    workout.notes = "Great workout";

    auto j = workout.to_json();

    EXPECT_EQ(j["name"], "Chest Day");
    EXPECT_EQ(j["duration_seconds"], 3600);
    EXPECT_EQ(j["notes"], "Great workout");
    EXPECT_TRUE(j.contains("completed_at"));
}

TEST(WorkoutSerializationUnit, SetsIncluded) {
    forge::Workout workout;
    workout.id = "w-1";
    workout.user_id = "user-1";
    workout.status = "completed";
    workout.started_at = std::chrono::system_clock::now();

    forge::ExerciseSet set;
    set.id = "set-1";
    set.workout_id = "w-1";
    set.exercise_id = "ex-1";
    set.set_order = 1;
    set.exercise_order = 1;
    set.set_type = "working";
    set.is_pr = false;
    set.reps = 10;
    set.weight_kg = 80.0;
    workout.sets.push_back(set);

    auto j = workout.to_json();

    EXPECT_EQ(j["sets"].size(), 1u);
    EXPECT_EQ(j["sets"][0]["id"], "set-1");
    EXPECT_EQ(j["sets"][0]["reps"], 10);
}

// ── NutritionLog::to_json() ────────────────────────────────────────

TEST(NutritionLogSerializationUnit, ContainsRequiredFields) {
    forge::NutritionLog log;
    log.id = "log-1";
    log.user_id = "user-1";
    log.food_name = "Chicken Breast";
    log.serving_size = 100.0;
    log.serving_unit = "g";
    log.quantity = 2.0;
    log.calories = 165.0;
    log.protein_g = 31.0;
    log.carbs_g = 0.0;
    log.fat_g = 3.6;
    log.is_custom = false;
    log.source = "manual";
    log.logged_at = std::chrono::system_clock::now();

    auto j = log.to_json();

    EXPECT_EQ(j["id"], "log-1");
    EXPECT_EQ(j["user_id"], "user-1");
    EXPECT_EQ(j["food_name"], "Chicken Breast");
    EXPECT_DOUBLE_EQ(j["serving_size"].get<double>(), 100.0);
    EXPECT_EQ(j["serving_unit"], "g");
    EXPECT_DOUBLE_EQ(j["quantity"].get<double>(), 2.0);
    EXPECT_DOUBLE_EQ(j["calories"].get<double>(), 165.0);
    EXPECT_DOUBLE_EQ(j["protein_g"].get<double>(), 31.0);
    EXPECT_FALSE(j["is_custom"].get<bool>());
    EXPECT_EQ(j["source"], "manual");
    EXPECT_TRUE(j.contains("logged_at"));
}

TEST(NutritionLogSerializationUnit, WithOptionalFields) {
    forge::NutritionLog log;
    log.id = "log-1";
    log.user_id = "user-1";
    log.food_name = "Granola Bar";
    log.serving_size = 1.0;
    log.serving_unit = "bar";
    log.quantity = 1.0;
    log.calories = 190.0;
    log.protein_g = 3.0;
    log.carbs_g = 29.0;
    log.fat_g = 7.0;
    log.is_custom = false;
    log.source = "usda";
    log.logged_at = std::chrono::system_clock::now();
    log.meal_type = "snack";
    log.brand = "Nature Valley";
    log.fiber_g = 2.0;
    log.sugar_g = 12.0;
    log.sodium_mg = 180.0;

    auto j = log.to_json();

    EXPECT_EQ(j["meal_type"], "snack");
    EXPECT_EQ(j["brand"], "Nature Valley");
    EXPECT_DOUBLE_EQ(j["fiber_g"].get<double>(), 2.0);
    EXPECT_DOUBLE_EQ(j["sugar_g"].get<double>(), 12.0);
    EXPECT_DOUBLE_EQ(j["sodium_mg"].get<double>(), 180.0);
}

TEST(NutritionLogSerializationUnit, OptionalFieldsAbsentWhenUnset) {
    forge::NutritionLog log;
    log.id = "log-1";
    log.user_id = "user-1";
    log.food_name = "Rice";
    log.serving_size = 100.0;
    log.serving_unit = "g";
    log.quantity = 1.0;
    log.calories = 130.0;
    log.protein_g = 2.7;
    log.carbs_g = 28.0;
    log.fat_g = 0.3;
    log.is_custom = false;
    log.source = "manual";
    log.logged_at = std::chrono::system_clock::now();

    auto j = log.to_json();

    EXPECT_FALSE(j.contains("meal_type"));
    EXPECT_FALSE(j.contains("brand"));
    EXPECT_FALSE(j.contains("fiber_g"));
    EXPECT_FALSE(j.contains("sugar_g"));
    EXPECT_FALSE(j.contains("sodium_mg"));
}

#pragma once
#include <vector>
#include <string>
#include <map>
#include <sstream>

namespace SystemPermissions {
// Categorías de permisos
enum Category {
    SYSTEM_ADMIN = 1,
    SENSOR_MANAGEMENT = 2,
    SECURITY_MONITORING = 3,
    REPORTS_ANALYTICS = 4,
    USER_MANAGEMENT = 5,
    AUDIT = 6
};

// Información de cada categoría
struct CategoryInfo {
    Category id;
    std::string name;
    std::string displayName;
    std::string description;
    std::string permissions; // Permisos separados por coma
};

// Definición completa de categorías
inline const std::vector<CategoryInfo> ALL_CATEGORIES = {
    {
        SYSTEM_ADMIN,
        "system_admin",
        "System Administration",
        "Manage system nodes, services, and monitor system health",
        "view_node_status,view_sensor_health,manage_nodes,restart_services,view_system_metrics"
    },
    {
        SENSOR_MANAGEMENT,
        "sensor_management",
        "Sensor Management",
        "Configure, calibrate and diagnose sensors",
        "View_sensor_status,view_raw_sensor_data,calibrate_sensors,diagnose_sensors"
    },
    {
        SECURITY_MONITORING,
        "security_monitoring",
        "Security Monitoring",
        "Monitor security alerts, alarms and generate incident reports",
        "View_realtime_alerts,receive_notifications,view_active_alarms,mark_reviewed,mark_in_progress,mark_false_alarm,View_alert_history,generate_incident_reports,filter_by_date,filter_by_sensor,filter_by_alarm_type,view_performance_stats"
    },
    {
        REPORTS_ANALYTICS,
        "reports_analytics",
        "Reports & Analytics",
        "Generate reports and analyze attendance data",
        "View_people_counting,view_attendance_data,generate_attendance_reports,export_csv,filter_by_event"
    },
    {
        USER_MANAGEMENT,
        "user_management",
        "User Management",
        "Manage users, roles and permissions",
        "Manage_users,manage_roles,assign_permissions,view_all_modules"
    },
    {
        AUDIT,
        "audit",
        "Audit & Compliance",
        "View system logs and generate audit reports",
        "View_audit_logs,view_access_logs,view_user_activity,generate_activity_reports"
    }
};

// Obtener categoría por ID
inline const CategoryInfo* getCategoryById(Category cat) {
    for (const auto& category : ALL_CATEGORIES) {
        if (category.id == cat) {
            return &category;
        }
    }
    return nullptr;
}

// Obtener categoría por nombre
inline const CategoryInfo* getCategoryByName(const std::string& name) {
    for (const auto& category : ALL_CATEGORIES) {
        if (category.name == name) {
            return &category;
        }
    }
    return nullptr;
}

// Convertir lista de categorías a permisos
inline std::string categoriesToPermissions(const std::vector<Category>& categories) {
    std::string result;
    for (size_t i = 0; i < categories.size(); i++) {
        const auto* cat = getCategoryById(categories[i]);
        if (cat) {
            if (!result.empty()) result += ",";
            result += cat->permissions;
        }
    }
    return result;
}

inline std::string categoryNamesToPermissions(const std::string& categoryNames) {
    std::string result;
    std::istringstream iss(categoryNames);
    std::string catName;

    while (std::getline(iss, catName, ',')) {
        // Trim
        catName.erase(0, catName.find_first_not_of(" \t"));
        catName.erase(catName.find_last_not_of(" \t") + 1);

        const auto* cat = getCategoryByName(catName);
        if (cat) {
            if (!result.empty()) result += ",";
            result += cat->permissions;
        }
    }
    return result;
}
}

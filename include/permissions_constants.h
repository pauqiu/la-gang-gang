#pragma once
#include <vector>
#include <string>
#include <map>

namespace SystemPermissions {
    enum Category {
        SYSTEM_ADMIN,
        SENSOR_MANAGEMENT,
        SECURITY_MONITORING,
        REPORTS_ANALYTICS,
        USER_MANAGEMENT,
        AUDIT
    };
    
    struct Permission {
        std::string id;
        std::string displayName;
        Category category;
    };
    
    inline const std::vector<Permission> ALL_PERMISSIONS = {
        // System Admin
        {"view_node_status", "View Node Status", SYSTEM_ADMIN},
        {"view_sensor_health", "View Sensor Health", SYSTEM_ADMIN},
        {"manage_nodes", "Manage Nodes", SYSTEM_ADMIN},
        {"restart_services", "Restart Services", SYSTEM_ADMIN},
        {"view_system_metrics", "View System Metrics", SYSTEM_ADMIN},
        
        // Sensor Management
        {"View_sensor_status", "View Sensor Status", SENSOR_MANAGEMENT},
        {"view_raw_sensor_data", "View Raw Sensor Data", SENSOR_MANAGEMENT},
        {"calibrate_sensors", "Calibrate Sensors", SENSOR_MANAGEMENT},
        {"diagnose_sensors", "Diagnose Sensors", SENSOR_MANAGEMENT},
        
        // Security Monitoring
        {"View_realtime_alerts", "View Realtime Alerts", SECURITY_MONITORING},
        {"receive_notifications", "Receive Notifications", SECURITY_MONITORING},
        {"view_active_alarms", "View Active Alarms", SECURITY_MONITORING},
        {"mark_reviewed", "Mark as Reviewed", SECURITY_MONITORING},
        {"mark_in_progress", "Mark In Progress", SECURITY_MONITORING},
        {"mark_false_alarm", "Mark False Alarm", SECURITY_MONITORING},
        {"View_alert_history", "View Alert History", SECURITY_MONITORING},
        {"generate_incident_reports", "Generate Incident Reports", SECURITY_MONITORING},
        {"filter_by_date", "Filter by Date", SECURITY_MONITORING},
        {"filter_by_sensor", "Filter by Sensor", SECURITY_MONITORING},
        {"filter_by_alarm_type", "Filter by Alarm Type", SECURITY_MONITORING},
        {"view_performance_stats", "View Performance Stats", SECURITY_MONITORING},
        
        // Reports & Analytics
        {"View_people_counting", "View People Counting", REPORTS_ANALYTICS},
        {"view_attendance_data", "View Attendance Data", REPORTS_ANALYTICS},
        {"generate_attendance_reports", "Generate Attendance Reports", REPORTS_ANALYTICS},
        {"export_csv", "Export to CSV", REPORTS_ANALYTICS},
        {"filter_by_event", "Filter by Event", REPORTS_ANALYTICS},
        
        // User Management
        {"Manage_users", "Manage Users", USER_MANAGEMENT},
        {"manage_roles", "Manage Roles", USER_MANAGEMENT},
        {"assign_permissions", "Assign Permissions", USER_MANAGEMENT},
        {"view_all_modules", "View All Modules", USER_MANAGEMENT},
        
        // Audit
        {"View_audit_logs", "View Audit Logs", AUDIT},
        {"view_access_logs", "View Access Logs", AUDIT},
        {"view_user_activity", "View User Activity", AUDIT},
        {"generate_activity_reports", "Generate Activity Reports", AUDIT}
    };
    
    inline std::string getCategoryName(Category cat) {
        switch(cat) {
            case SYSTEM_ADMIN: return "System Administration";
            case SENSOR_MANAGEMENT: return "Sensor Management";
            case SECURITY_MONITORING: return "Security Monitoring";
            case REPORTS_ANALYTICS: return "Reports & Analytics";
            case USER_MANAGEMENT: return "User Management";
            case AUDIT: return "Audit & Compliance";
            default: return "Unknown";
        }
    }

    inline std::vector<Permission> getPermissionsByCategory(Category cat) {
        std::vector<Permission> result;
        for (const auto& perm : ALL_PERMISSIONS) {
            if (perm.category == cat) {
                result.push_back(perm);
            }
        }
        return result;
    }
}

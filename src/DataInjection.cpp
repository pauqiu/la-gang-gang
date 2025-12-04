#include "DataInjection.h"
#include "encryptation.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <tuple>
#include <string>

namespace {
    static bool fileExists(const std::string &filename) {
        std::ifstream f(filename.c_str());
        return f.good();
    }

    static void injectSampleUsers(FileSystem &fs) {
        const std::string usersFile = "Users.txt";

        fs.createFile(usersFile);

        std::vector<std::tuple<std::string, std::string, std::string>> sampleUsers = {
            {"admin01", Encryptation::encryptPassword("sysadmin_01"), "admin_sistema"},
            {"admin02", Encryptation::encryptPassword("admin123+"), "admin_sistema"},
            {"technician01", Encryptation::encryptPassword("sensorGOD123"), "tecnico_sensores"},
            {"technician02",   Encryptation::encryptPassword("technician15"), "tecnico_sensores"},
            {"guard01",   Encryptation::encryptPassword("security$"), "oficial_seguridad"},
            {"guard02",   Encryptation::encryptPassword("guardaSensor*"), "oficial_seguridad"},
            {"supervisor01",   Encryptation::encryptPassword("visor_super777"), "supervisor_seguridad"},
            {"supervisor02",   Encryptation::encryptPassword("gang_gang"), "supervisor_seguridad"},
            {"analyst01",   Encryptation::encryptPassword("bull010203"), "analista_negocios"},
            {"analyst02",   Encryptation::encryptPassword("$BSNSSANLST"), "analista_negocios"},
            {"genAdmin01",   Encryptation::encryptPassword("contraseña123"), "admin_general"},
            {"genAdmin02",   Encryptation::encryptPassword("$generalAD!"), "admin_general"},
            {"auditor01",   Encryptation::encryptPassword("Auditor467"), "auditor"},
            {"auditor02",   Encryptation::encryptPassword("jg12GD15$$"), "auditor"}
        };

        std::string usersContent;
        for (const auto &u : sampleUsers) {
            usersContent += std::get<0>(u) + ":" + std::get<1>(u) + ":" + std::get<2>(u) + "\n";
        }

        fs.overwriteFile(usersFile, usersContent);
        std::cout << "Users.txt poblado con " << sampleUsers.size() << " usuarios." << std::endl;
    }

    static void injectSampleRoles(FileSystem &fs) {
        const std::string rolesFile = "roles.txt";

        fs.createFile(rolesFile);

        std::vector<std::tuple<int, std::string, std::string>> sampleRoles = {
            {1, "admin_sistema", "view_node_status,view_sensor_health,manage_nodes,restart_services,view_system_metrics"},
            {2, "tecnico_sensores",  "View_sensor_status,view_sensor_health,view_raw_sensor_data,calibrate_sensors,diagnose_sensors"},
            {3, "oficial_seguridad", "View_realtime_alerts,receive_notifications,view_active_alarms,mark_reviewed,mark_in_progress, mark_false_alarm"},
            {4, "supervisor_seguridad",  "View_alert_history,generate_incident_reports,filter_by_date,filter_by_sensor,filter_by_alarm_type,view_system_metrics, view_performance_stats"},
            {5, "analista_negocios",  "View_people_counting,view_attendance_data,generate_attendance_reports,export_csv, filter_by_event,filter_by_date"},
            {6, "admin_general",  "Manage_users,manage_roles,assign_permissions,view_all_modules,view_audit_logs"},
            {7, "auditor",  "View_audit_logs,view_access_logs,view_user_activity,generate_activity_reports"}
        };

        std::string rolesContent;
        rolesContent += "# Roles System File\n";
        rolesContent += "# Format: id;role_name;permissions\n";
        rolesContent += "# Permissions are comma-separated (e.g., read,write)\n";
        for (const auto &r : sampleRoles) {
            rolesContent += std::to_string(std::get<0>(r)) + ";" + std::get<1>(r) + ";" + std::get<2>(r) + "\n";
        }

        fs.overwriteFile(rolesFile, rolesContent);
        std::cout << "roles.txt poblado con " << sampleRoles.size() << " roles." << std::endl;
    }

    static void injectSamplePermissions(FileSystem &fs) {
        const std::string permsFile = "perms.txt";

        fs.createFile(permsFile);

        std::vector<std::tuple<int, std::string>> samplePermissions = {
            {1, "read"},
            {2, "write"},
            {3, "delete"}
        };

        std::string permsContent;
        for (const auto &p : samplePermissions) {
            permsContent += std::to_string(std::get<0>(p)) + ";" + std::get<1>(p) + "\n";
        }

        fs.overwriteFile(permsFile, permsContent);
        std::cout << "perms.txt poblado con " << samplePermissions.size() << " permisos." << std::endl;
    }

    static void injectSampleData(FileSystem &fs) {
        std::cout << "Primer inicio detectado: inyectando datos de prueba..." << std::endl;
        injectSampleUsers(fs);
        injectSampleRoles(fs);
        injectSamplePermissions(fs);
    }
}

namespace DataInjection {
    bool ensureSampleData(FileSystem &fs, const std::string &diskName) {
        if (!fileExists(diskName)) {
            injectSampleData(fs);
            return true;
        }
        return false;
    }

    void injectSampleData(FileSystem &fs) {
        ::injectSampleData(fs);
    }
}

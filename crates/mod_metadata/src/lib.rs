use std::collections::HashMap;

use error::{CoreError, Result};
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum ModSide {
    Required,
    Optional,
    Unsupported,
    Unknown,
}

impl ModSide {
    pub fn from_curseforge(value: i64) -> Self {
        match value {
            1 => ModSide::Required,
            2 => ModSide::Optional,
            3 => ModSide::Unsupported,
            _ => ModSide::Unknown,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum ModVersionType {
    Release,
    Beta,
    Alpha,
    Unknown,
}

impl ModVersionType {
    pub fn from_curseforge(value: i64) -> Self {
        match value {
            1 => ModVersionType::Release,
            2 => ModVersionType::Beta,
            3 => ModVersionType::Alpha,
            _ => ModVersionType::Unknown,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum ModLoader {
    Forge,
    NeoForge,
    Fabric,
    Quilt,
    LiteLoader,
    Cauldron,
    Rift,
    Babric,
    Bta,
    LegacyFabric,
    Ornithe,
    DataPack,
    Unknown,
}

impl ModLoader {
    pub fn from_modrinth(name: &str) -> Self {
        match name.to_lowercase().as_str() {
            "forge" => ModLoader::Forge,
            "neoforge" => ModLoader::NeoForge,
            "fabric" => ModLoader::Fabric,
            "quilt" => ModLoader::Quilt,
            "liteloader" => ModLoader::LiteLoader,
            "cauldron" => ModLoader::Cauldron,
            "rift" => ModLoader::Rift,
            "babric" => ModLoader::Babric,
            "bta" => ModLoader::Bta,
            "legacyfabric" => ModLoader::LegacyFabric,
            "ornithe" => ModLoader::Ornithe,
            "datapack" => ModLoader::DataPack,
            _ => ModLoader::Unknown,
        }
    }

    pub fn from_curseforge_name(name: &str) -> Self {
        match name {
            "Forge" => ModLoader::Forge,
            "Cauldron" => ModLoader::Cauldron,
            "LiteLoader" => ModLoader::LiteLoader,
            "Fabric" => ModLoader::Fabric,
            "Quilt" => ModLoader::Quilt,
            "NeoForge" => ModLoader::NeoForge,
            _ => ModLoader::Unknown,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum DependencyType {
    Required,
    Optional,
    Incompatible,
    Embedded,
    Tool,
    Include,
    Unknown,
}

impl DependencyType {
    pub fn from_curseforge(value: i64) -> Self {
        match value {
            1 => DependencyType::Embedded,
            2 => DependencyType::Optional,
            3 => DependencyType::Required,
            4 => DependencyType::Tool,
            5 => DependencyType::Incompatible,
            6 => DependencyType::Include,
            _ => DependencyType::Unknown,
        }
    }

    pub fn from_modrinth(name: &str) -> Self {
        match name {
            "required" => DependencyType::Required,
            "optional" => DependencyType::Optional,
            "incompatible" => DependencyType::Incompatible,
            "embedded" => DependencyType::Embedded,
            _ => DependencyType::Unknown,
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ModAuthor {
    pub name: String,
    #[serde(default)]
    pub url: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ModDependency {
    pub project_id: String,
    #[serde(default)]
    pub version_id: String,
    #[serde(default)]
    pub file_name: String,
    pub dep_type: DependencyType,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ModHash {
    pub hash_type: String,
    pub hash: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ModDonation {
    #[serde(default)]
    pub id: String,
    #[serde(default)]
    pub platform: String,
    pub url: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ModLinks {
    #[serde(default)]
    pub website_url: String,
    #[serde(default)]
    pub issues_url: String,
    #[serde(default)]
    pub source_url: String,
    #[serde(default)]
    pub wiki_url: String,
    #[serde(default)]
    pub discord_url: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ModProject {
    pub project_id: String,
    pub name: String,
    pub slug: String,
    #[serde(default)]
    pub description: String,
    #[serde(default)]
    pub logo_url: String,
    #[serde(default)]
    pub website_url: String,
    #[serde(default)]
    pub authors: Vec<ModAuthor>,
    #[serde(default)]
    pub client_side: ModSide,
    #[serde(default)]
    pub server_side: ModSide,
    #[serde(default)]
    pub links: ModLinks,
    #[serde(default)]
    pub body: String,
    #[serde(default)]
    pub status: String,
    #[serde(default)]
    pub donations: Vec<ModDonation>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ModVersion {
    pub version_id: String,
    pub project_id: String,
    pub display_name: String,
    #[serde(default)]
    pub version_number: String,
    pub version_type: ModVersionType,
    #[serde(default)]
    pub game_versions: Vec<String>,
    #[serde(default)]
    pub loaders: Vec<ModLoader>,
    #[serde(default)]
    pub download_url: String,
    #[serde(default)]
    pub file_name: String,
    #[serde(default)]
    pub date: String,
    #[serde(default)]
    pub hash: Option<ModHash>,
    #[serde(default)]
    pub dependencies: Vec<ModDependency>,
    #[serde(default)]
    pub changelog: String,
    #[serde(default)]
    pub is_preferred: bool,
    #[serde(default)]
    pub client_side: ModSide,
    #[serde(default)]
    pub server_side: ModSide,
}

pub fn parse_modrinth_project(json: &str) -> Result<ModProject> {
    let raw: serde_json::Value = serde_json::from_str(json).map_err(CoreError::Json)?;

    let project_id = raw["id"].as_str().or_else(|| raw["project_id"].as_str()).unwrap_or("").to_string();

    let authors = extract_modrinth_authors(&raw);

    let client_side = parse_modrinth_side(raw["client_side"].as_str().unwrap_or("unknown"));
    let server_side = parse_modrinth_side(raw["server_side"].as_str().unwrap_or("unknown"));

    let links = ModLinks {
        website_url: raw["website_url"].as_str().unwrap_or("").to_string(),
        issues_url: raw["issues_url"].as_str().unwrap_or("").to_string(),
        source_url: raw["source_url"].as_str().unwrap_or("").to_string(),
        wiki_url: raw["wiki_url"].as_str().unwrap_or("").to_string(),
        discord_url: raw["discord_url"].as_str().unwrap_or("").to_string(),
    };

    let donations = raw["donation_urls"]
        .as_array()
        .map(|arr| {
            arr.iter()
                .map(|d| ModDonation {
                    id: d["id"].as_str().unwrap_or("").to_string(),
                    platform: d["platform"].as_str().unwrap_or("").to_string(),
                    url: d["url"].as_str().unwrap_or("").to_string(),
                })
                .collect()
        })
        .unwrap_or_default();

    Ok(ModProject {
        project_id,
        name: raw["title"].as_str().unwrap_or("").to_string(),
        slug: raw["slug"].as_str().unwrap_or("").to_string(),
        description: raw["description"].as_str().unwrap_or("").to_string(),
        logo_url: raw["icon_url"].as_str().unwrap_or("").to_string(),
        website_url: format!("https://modrinth.com/mod/{}", raw["slug"].as_str().unwrap_or("")),
        authors,
        client_side,
        server_side,
        links,
        body: raw["body"].as_str().unwrap_or("").to_string(),
        status: raw["status"].as_str().unwrap_or("").to_string(),
        donations,
    })
}

pub fn parse_modrinth_version(json: &str) -> Result<ModVersion> {
    let raw: serde_json::Value = serde_json::from_str(json).map_err(CoreError::Json)?;

    let loaders: Vec<ModLoader> = raw["loaders"]
        .as_array()
        .map(|arr| arr.iter().filter_map(|v| v.as_str().map(ModLoader::from_modrinth)).collect())
        .unwrap_or_default();

    let game_versions: Vec<String> = raw["game_versions"]
        .as_array()
        .map(|arr| arr.iter().filter_map(|v| v.as_str().map(String::from)).collect())
        .unwrap_or_default();

    let hash = raw["files"]
        .as_array()
        .and_then(|files| files.first())
        .and_then(|f| f["hashes"].as_object())
        .and_then(|hashes| {
            hashes.iter().next().map(|(algo, val)| ModHash {
                hash_type: algo.clone(),
                hash: val.as_str().unwrap_or("").to_string(),
            })
        });

    let dependencies = raw["dependencies"]
        .as_array()
        .map(|arr| {
            arr.iter()
                .filter_map(|d| {
                    let project_id = d["project_id"].as_str()?.to_string();
                    let version_id = d["version_id"].as_str().unwrap_or("").to_string();
                    let dep_type = d["dependency_type"]
                        .as_str()
                        .map(DependencyType::from_modrinth)
                        .unwrap_or(DependencyType::Unknown);
                    Some(ModDependency {
                        project_id,
                        version_id,
                        file_name: String::new(),
                        dep_type,
                    })
                })
                .collect()
        })
        .unwrap_or_default();

    let version_type = match raw["version_type"].as_str().unwrap_or("release") {
        "release" => ModVersionType::Release,
        "beta" => ModVersionType::Beta,
        "alpha" => ModVersionType::Alpha,
        _ => ModVersionType::Unknown,
    };

    let client_side = parse_modrinth_side("unknown");
    let server_side = parse_modrinth_side("unknown");

    Ok(ModVersion {
        version_id: raw["id"].as_str().unwrap_or("").to_string(),
        project_id: raw["project_id"].as_str().unwrap_or("").to_string(),
        display_name: raw["name"].as_str().unwrap_or("").to_string(),
        version_number: raw["version_number"].as_str().unwrap_or("").to_string(),
        version_type,
        game_versions,
        loaders,
        download_url: raw["files"]
            .as_array()
            .and_then(|files| files.first())
            .and_then(|f| f["url"].as_str())
            .unwrap_or("")
            .to_string(),
        file_name: raw["files"]
            .as_array()
            .and_then(|files| files.first())
            .and_then(|f| f["filename"].as_str())
            .unwrap_or("")
            .to_string(),
        date: raw["date_published"].as_str().unwrap_or("").to_string(),
        hash,
        dependencies,
        changelog: raw["changelog"].as_str().unwrap_or("").to_string(),
        is_preferred: raw["files"]
            .as_array()
            .and_then(|files| files.first())
            .and_then(|f| f["primary"].as_bool())
            .unwrap_or(false),
        client_side,
        server_side,
    })
}

pub fn parse_curseforge_project(json: &str) -> Result<ModProject> {
    let raw: serde_json::Value = serde_json::from_str(json).map_err(CoreError::Json)?;

    let authors = raw["authors"]
        .as_array()
        .map(|arr| {
            arr.iter()
                .map(|a| ModAuthor {
                    name: a["name"].as_str().unwrap_or("").to_string(),
                    url: a["url"].as_str().unwrap_or("").to_string(),
                })
                .collect()
        })
        .unwrap_or_default();

    let logo_url = raw["logo"]
        .as_object()
        .and_then(|logo| logo.get("thumbnailUrl").or_else(|| logo.get("url")).and_then(|v| v.as_str()))
        .unwrap_or("")
        .to_string();

    let links = ModLinks {
        website_url: raw["links"]
            .as_object()
            .and_then(|l| l["websiteUrl"].as_str())
            .unwrap_or("")
            .to_string(),
        issues_url: raw["links"]
            .as_object()
            .and_then(|l| l["issuesUrl"].as_str())
            .unwrap_or("")
            .to_string(),
        source_url: raw["links"]
            .as_object()
            .and_then(|l| l["sourceUrl"].as_str())
            .unwrap_or("")
            .to_string(),
        wiki_url: raw["links"]
            .as_object()
            .and_then(|l| l["wikiUrl"].as_str())
            .unwrap_or("")
            .to_string(),
        discord_url: String::new(),
    };

    Ok(ModProject {
        project_id: raw["id"]
            .as_i64()
            .map(|v| v.to_string())
            .or_else(|| raw["id"].as_str().map(String::from))
            .unwrap_or_default(),
        name: raw["name"].as_str().unwrap_or("").to_string(),
        slug: raw["slug"].as_str().unwrap_or("").to_string(),
        description: raw["summary"].as_str().unwrap_or("").to_string(),
        logo_url,
        website_url: links.website_url.clone(),
        authors,
        client_side: ModSide::Unknown,
        server_side: ModSide::Unknown,
        links,
        body: raw["description"].as_str().unwrap_or("").to_string(),
        status: String::new(),
        donations: Vec::new(),
    })
}

pub fn parse_curseforge_version(json: &str) -> Result<ModVersion> {
    let raw: serde_json::Value = serde_json::from_str(json).map_err(CoreError::Json)?;

    let mut game_versions = Vec::new();
    let mut loaders = Vec::new();

    if let Some(versions) = raw["gameVersions"].as_array() {
        for v in versions {
            if let Some(name) = v.as_str() {
                match ModLoader::from_curseforge_name(name) {
                    ModLoader::Unknown => game_versions.push(name.to_string()),
                    loader => loaders.push(loader),
                }
            }
        }
    }

    let hash = raw["hashes"].as_array().and_then(|hashes| hashes.first()).map(|h| ModHash {
        hash_type: match h["algo"].as_i64().unwrap_or(0) {
            1 => "sha1".to_string(),
            2 => "sha256".to_string(),
            _ => "md5".to_string(),
        },
        hash: h["value"].as_str().unwrap_or("").to_string(),
    });

    let dependencies = raw["dependencies"]
        .as_array()
        .map(|arr| {
            arr.iter()
                .map(|d| ModDependency {
                    project_id: d["modId"].as_i64().map(|v| v.to_string()).unwrap_or_default(),
                    version_id: String::new(),
                    file_name: String::new(),
                    dep_type: d["relationType"]
                        .as_i64()
                        .map(DependencyType::from_curseforge)
                        .unwrap_or(DependencyType::Unknown),
                })
                .collect()
        })
        .unwrap_or_default();

    let version_type = raw["releaseType"]
        .as_i64()
        .map(ModVersionType::from_curseforge)
        .unwrap_or(ModVersionType::Unknown);

    Ok(ModVersion {
        version_id: raw["id"]
            .as_i64()
            .map(|v| v.to_string())
            .or_else(|| raw["id"].as_str().map(String::from))
            .unwrap_or_default(),
        project_id: raw["modId"]
            .as_i64()
            .map(|v| v.to_string())
            .or_else(|| raw["modId"].as_str().map(String::from))
            .unwrap_or_default(),
        display_name: raw["displayName"].as_str().unwrap_or("").to_string(),
        version_number: String::new(),
        version_type,
        game_versions,
        loaders,
        download_url: raw["downloadUrl"].as_str().unwrap_or("").to_string(),
        file_name: raw["fileName"].as_str().unwrap_or("").to_string(),
        date: raw["fileDate"].as_str().unwrap_or("").to_string(),
        hash,
        dependencies,
        changelog: raw["changelog"].as_str().unwrap_or("").to_string(),
        is_preferred: false,
        client_side: ModSide::Unknown,
        server_side: ModSide::Unknown,
    })
}

fn parse_modrinth_side(value: &str) -> ModSide {
    match value {
        "required" => ModSide::Required,
        "optional" => ModSide::Optional,
        "unsupported" => ModSide::Unsupported,
        _ => ModSide::Unknown,
    }
}

fn extract_modrinth_authors(raw: &serde_json::Value) -> Vec<ModAuthor> {
    if let Some(authors) = raw["authors"].as_array() {
        return authors
            .iter()
            .map(|a| ModAuthor {
                name: a["name"].as_str().unwrap_or("").to_string(),
                url: a["url"].as_str().unwrap_or("").to_string(),
            })
            .collect();
    }
    if let Some(author) = raw["author"].as_str() {
        return vec![ModAuthor {
            name: author.to_string(),
            url: String::new(),
        }];
    }
    Vec::new()
}

pub fn filter_versions_by_loaders(versions: &[ModVersion], required_loaders: &[ModLoader]) -> Vec<ModVersion> {
    if required_loaders.is_empty() {
        return versions.to_vec();
    }
    versions
        .iter()
        .filter(|v| required_loaders.iter().any(|loader| v.loaders.contains(loader)))
        .cloned()
        .collect()
}

pub fn filter_versions_by_mc_version(versions: &[ModVersion], mc_version: &str) -> Vec<ModVersion> {
    versions
        .iter()
        .filter(|v| v.game_versions.iter().any(|v| v == mc_version))
        .cloned()
        .collect()
}

pub fn sort_versions_by_date(versions: &mut [ModVersion]) {
    versions.sort_by(|a, b| b.date.cmp(&a.date));
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_modrinth_side_parsing() {
        assert_eq!(parse_modrinth_side("required"), ModSide::Required);
        assert_eq!(parse_modrinth_side("optional"), ModSide::Optional);
        assert_eq!(parse_modrinth_side("unsupported"), ModSide::Unsupported);
        assert_eq!(parse_modrinth_side("unknown"), ModSide::Unknown);
    }

    #[test]
    fn test_curseforge_version_type() {
        assert_eq!(ModVersionType::from_curseforge(1), ModVersionType::Release);
        assert_eq!(ModVersionType::from_curseforge(2), ModVersionType::Beta);
        assert_eq!(ModVersionType::from_curseforge(3), ModVersionType::Alpha);
        assert_eq!(ModVersionType::from_curseforge(99), ModVersionType::Unknown);
    }

    #[test]
    fn test_dependency_type_curseforge() {
        assert_eq!(DependencyType::from_curseforge(1), DependencyType::Embedded);
        assert_eq!(DependencyType::from_curseforge(2), DependencyType::Optional);
        assert_eq!(DependencyType::from_curseforge(3), DependencyType::Required);
        assert_eq!(DependencyType::from_curseforge(4), DependencyType::Tool);
        assert_eq!(DependencyType::from_curseforge(5), DependencyType::Incompatible);
    }

    #[test]
    fn test_dependency_type_modrinth() {
        assert_eq!(DependencyType::from_modrinth("required"), DependencyType::Required);
        assert_eq!(DependencyType::from_modrinth("optional"), DependencyType::Optional);
        assert_eq!(DependencyType::from_modrinth("incompatible"), DependencyType::Incompatible);
    }

    #[test]
    fn test_mod_loader_from_modrinth() {
        assert_eq!(ModLoader::from_modrinth("forge"), ModLoader::Forge);
        assert_eq!(ModLoader::from_modrinth("fabric"), ModLoader::Fabric);
        assert_eq!(ModLoader::from_modrinth("quilt"), ModLoader::Quilt);
        assert_eq!(ModLoader::from_modrinth("neoforge"), ModLoader::NeoForge);
        assert_eq!(ModLoader::from_modrinth("unknown"), ModLoader::Unknown);
    }

    #[test]
    fn test_filter_versions_by_loader() {
        let versions = vec![
            ModVersion {
                version_id: "1".into(),
                project_id: "p".into(),
                display_name: "v1".into(),
                version_number: "1.0".into(),
                version_type: ModVersionType::Release,
                game_versions: vec![],
                loaders: vec![ModLoader::Forge],
                download_url: String::new(),
                file_name: String::new(),
                date: String::new(),
                hash: None,
                dependencies: vec![],
                changelog: String::new(),
                is_preferred: false,
                client_side: ModSide::Unknown,
                server_side: ModSide::Unknown,
            },
            ModVersion {
                version_id: "2".into(),
                project_id: "p".into(),
                display_name: "v2".into(),
                version_number: "2.0".into(),
                version_type: ModVersionType::Release,
                game_versions: vec![],
                loaders: vec![ModLoader::Fabric],
                download_url: String::new(),
                file_name: String::new(),
                date: String::new(),
                hash: None,
                dependencies: vec![],
                changelog: String::new(),
                is_preferred: false,
                client_side: ModSide::Unknown,
                server_side: ModSide::Unknown,
            },
        ];

        let filtered = filter_versions_by_loaders(&versions, &[ModLoader::Forge]);
        assert_eq!(filtered.len(), 1);
        assert_eq!(filtered[0].version_id, "1");
    }

    #[test]
    fn test_filter_versions_by_mc_version() {
        let versions = vec![ModVersion {
            version_id: "1".into(),
            project_id: "p".into(),
            display_name: "v1".into(),
            version_number: "1.0".into(),
            version_type: ModVersionType::Release,
            game_versions: vec!["1.20.1".into(), "1.20.2".into()],
            loaders: vec![],
            download_url: String::new(),
            file_name: String::new(),
            date: String::new(),
            hash: None,
            dependencies: vec![],
            changelog: String::new(),
            is_preferred: false,
            client_side: ModSide::Unknown,
            server_side: ModSide::Unknown,
        }];

        let filtered = filter_versions_by_mc_version(&versions, "1.20.1");
        assert_eq!(filtered.len(), 1);

        let filtered = filter_versions_by_mc_version(&versions, "1.19.4");
        assert_eq!(filtered.len(), 0);
    }

    #[test]
    fn test_parse_modrinth_project() {
        let json = r#"{
            "id": "abc123",
            "title": "Test Mod",
            "slug": "test-mod",
            "description": "A test mod",
            "icon_url": "https://example.com/icon.png",
            "client_side": "required",
            "server_side": "optional",
            "authors": [{"name": "Author1", "url": "https://example.com"}],
            "body": "Full description",
            "status": "listed"
        }"#;

        let project = parse_modrinth_project(json).unwrap();
        assert_eq!(project.project_id, "abc123");
        assert_eq!(project.name, "Test Mod");
        assert_eq!(project.slug, "test-mod");
        assert_eq!(project.client_side, ModSide::Required);
        assert_eq!(project.server_side, ModSide::Optional);
        assert_eq!(project.authors.len(), 1);
        assert_eq!(project.authors[0].name, "Author1");
    }

    #[test]
    fn test_parse_modrinth_version() {
        let json = r#"{
            "id": "v1",
            "project_id": "p1",
            "name": "Version 1.0",
            "version_number": "1.0.0",
            "version_type": "release",
            "game_versions": ["1.20.1"],
            "loaders": ["forge", "fabric"],
            "date_published": "2024-01-01",
            "files": [{
                "url": "https://example.com/mod.jar",
                "filename": "mod-1.0.0.jar",
                "primary": true,
                "hashes": {"sha1": "abc123"}
            }],
            "dependencies": []
        }"#;

        let version = parse_modrinth_version(json).unwrap();
        assert_eq!(version.version_id, "v1");
        assert_eq!(version.display_name, "Version 1.0");
        assert_eq!(version.version_type, ModVersionType::Release);
        assert_eq!(version.game_versions, vec!["1.20.1"]);
        assert!(version.loaders.contains(&ModLoader::Forge));
        assert!(version.loaders.contains(&ModLoader::Fabric));
        assert_eq!(version.file_name, "mod-1.0.0.jar");
        assert!(version.hash.is_some());
        assert_eq!(version.hash.as_ref().unwrap().hash_type, "sha1");
    }

    #[test]
    fn test_parse_curseforge_project() {
        let json = r#"{
            "id": 12345,
            "name": "CF Mod",
            "slug": "cf-mod",
            "summary": "Summary text",
            "description": "Full description",
            "authors": [{"name": "CF Author", "url": "https://cf.example.com"}],
            "logo": {"thumbnailUrl": "https://example.com/logo.png", "url": "https://example.com/logo.png"},
            "links": {"websiteUrl": "https://example.com", "issuesUrl": "https://example.com/issues", "sourceUrl": "https://example.com/src", "wikiUrl": ""}
        }"#;

        let project = parse_curseforge_project(json).unwrap();
        assert_eq!(project.project_id, "12345");
        assert_eq!(project.name, "CF Mod");
        assert_eq!(project.slug, "cf-mod");
        assert_eq!(project.description, "Summary text");
        assert_eq!(project.authors[0].name, "CF Author");
        assert_eq!(project.links.issues_url, "https://example.com/issues");
    }

    #[test]
    fn test_parse_curseforge_version() {
        let json = r#"{
            "id": 67890,
            "modId": 12345,
            "displayName": "Mod v2.0",
            "downloadUrl": "https://example.com/mod.jar",
            "fileName": "mod-2.0.jar",
            "fileDate": "2024-06-01",
            "releaseType": 1,
            "gameVersions": ["1.20.1", "Forge"],
            "hashes": [{"algo": 2, "value": "def456"}],
            "dependencies": [{"modId": 99999, "relationType": 3}]
        }"#;

        let version = parse_curseforge_version(json).unwrap();
        assert_eq!(version.version_id, "67890");
        assert_eq!(version.project_id, "12345");
        assert_eq!(version.display_name, "Mod v2.0");
        assert_eq!(version.version_type, ModVersionType::Release);
        assert_eq!(version.game_versions, vec!["1.20.1"]);
        assert!(version.loaders.contains(&ModLoader::Forge));
        assert_eq!(version.hash.as_ref().unwrap().hash_type, "sha256");
        assert_eq!(version.dependencies.len(), 1);
        assert_eq!(version.dependencies[0].dep_type, DependencyType::Required);
    }
}

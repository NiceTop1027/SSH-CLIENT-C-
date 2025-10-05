# Interface Contract: IProfileStorage

**Component**: Connection Profile Storage
**Version**: 1.0
**Date**: 2025-10-05

## Purpose
Manages persistence of connection profiles to local file system, including CRUD operations, validation, and file I/O.

## Interface Definition

```cpp
class IProfileStorage {
public:
    virtual ~IProfileStorage() = default;

    // CRUD operations
    virtual Result<ProfileId> createProfile(const ConnectionProfile& profile) = 0;
    virtual Result<ConnectionProfile> readProfile(ProfileId id) const = 0;
    virtual Result<void> updateProfile(const ConnectionProfile& profile) = 0;
    virtual Result<void> deleteProfile(ProfileId id) = 0;

    // Bulk operations
    virtual Result<std::vector<ConnectionProfile>> listProfiles() const = 0;
    virtual Result<std::vector<ConnectionProfile>> listProfilesInFolder(const std::string& folder) const = 0;
    virtual Result<std::vector<std::string>> listFolders() const = 0;

    // Search and filter
    virtual std::vector<ConnectionProfile> searchProfiles(const std::string& query) const = 0;
    virtual std::vector<ConnectionProfile> filterByFolder(const std::string& folder) const = 0;

    // Import/Export
    virtual Result<void> exportProfile(ProfileId id, const std::string& filePath) const = 0;
    virtual Result<ProfileId> importProfile(const std::string& filePath) = 0;

    // Validation
    virtual ValidationResult validateProfile(const ConnectionProfile& profile) const = 0;
};
```

## Types

### ProfileId
```cpp
using ProfileId = QUuid;
```

### ValidationResult
```cpp
struct ValidationResult {
    bool isValid;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};
```

## Method Contracts

### createProfile(profile)
**Purpose**: Creates new connection profile and persists to disk

**Preconditions**:
- `profile` passes validation
- `profile.name` is unique
- Storage directory exists or can be created

**Postconditions** (success):
- Profile assigned new UUID
- Profile saved to `~/.ssh-client/profiles/{uuid}.json`
- Returns ProfileId

**Errors**:
- `VALIDATION_FAILED`: Profile validation failed
- `DUPLICATE_NAME`: Profile with same name exists
- `STORAGE_ERROR`: File system error (permissions, disk full)

**Side Effects**:
- Creates profile directory if doesn't exist
- `profile.createdAt` and `profile.lastModified` set to current time

---

### readProfile(id)
**Purpose**: Loads connection profile from disk

**Preconditions**:
- `id` is valid UUID format

**Postconditions** (success):
- Profile loaded from file
- Returns ConnectionProfile

**Errors**:
- `PROFILE_NOT_FOUND`: No profile file with that ID
- `PARSE_ERROR`: File exists but contains invalid JSON
- `VALIDATION_FAILED`: Loaded profile fails validation (corrupted)

**Performance**: < 10ms for typical profile

---

### updateProfile(profile)
**Purpose**: Updates existing profile on disk

**Preconditions**:
- `profile.id` exists in storage
- Updated `profile` passes validation
- If name changed, new name is unique

**Postconditions** (success):
- Profile file overwritten with updated data
- `profile.lastModified` updated to current time

**Errors**:
- `PROFILE_NOT_FOUND`: Profile ID doesn't exist
- `VALIDATION_FAILED`: Updated profile invalid
- `DUPLICATE_NAME`: Name collision with another profile
- `STORAGE_ERROR`: File system error

**Side Effects**:
- Original profile file backed up before write (optional)

---

### deleteProfile(id)
**Purpose**: Removes profile from storage

**Preconditions**:
- `id` is valid UUID format

**Postconditions** (success):
- Profile file deleted from disk
- Associated credentials removed from OS keychain

**Errors**:
- `PROFILE_NOT_FOUND`: Profile ID doesn't exist
- `STORAGE_ERROR`: File deletion failed

**Side Effects**:
- Credential cleanup may fail independently (logged but doesn't fail delete)

---

### listProfiles()
**Purpose**: Returns all stored connection profiles

**Preconditions**: None

**Postconditions**:
- Returns vector of all profiles
- Empty vector if no profiles
- Profiles sorted by `lastUsed` descending (most recent first)

**Errors**:
- `STORAGE_ERROR`: Directory read error

**Performance**: O(n) where n = number of profiles, lazy load option for large sets

---

### listProfilesInFolder(folder)
**Purpose**: Returns profiles in specific folder

**Preconditions**:
- `folder` is non-empty string

**Postconditions**:
- Returns profiles where `profile.folder == folder`
- Empty vector if folder doesn't exist or has no profiles
- Sorted by name

**Errors**:
- `STORAGE_ERROR`: Directory read error

---

### listFolders()
**Purpose**: Returns list of all folder names used in profiles

**Preconditions**: None

**Postconditions**:
- Returns unique folder names from all profiles
- Empty vector if no folders used
- Sorted alphabetically

**Errors**:
- `STORAGE_ERROR`: Directory read error

---

### searchProfiles(query)
**Purpose**: Searches profiles by name, hostname, username, or folder

**Preconditions**:
- `query` is non-empty string

**Postconditions**:
- Returns profiles matching query (case-insensitive)
- Matches checked against: name, hostname, username, folder
- Sorted by relevance (exact match > prefix match > contains)

**Errors**: None (returns empty vector on error)

**Performance**: O(n) linear search acceptable for typical profile counts

---

### filterByFolder(folder)
**Purpose**: Filters profiles by exact folder match

**Preconditions**: None

**Postconditions**:
- Returns profiles where `profile.folder == folder`
- Empty vector if no matches

**Errors**: None (returns empty vector on error)

---

### exportProfile(id, filePath)
**Purpose**: Exports single profile to external file

**Preconditions**:
- `id` exists in storage
- `filePath` is writable location

**Postconditions** (success):
- Profile written to file at `filePath` in JSON format
- Credentials NOT included (security)

**Errors**:
- `PROFILE_NOT_FOUND`: Profile ID doesn't exist
- `STORAGE_ERROR`: Cannot write to filePath

**Security Note**: Exported files contain no credentials

---

### importProfile(filePath)
**Purpose**: Imports profile from external file

**Preconditions**:
- File at `filePath` exists and is readable
- File contains valid profile JSON

**Postconditions** (success):
- Profile loaded and validated
- New UUID assigned (original ID ignored)
- If name conflicts, new name generated (e.g., "Name (2)")
- Profile saved to storage
- Returns new ProfileId

**Errors**:
- `FILE_NOT_FOUND`: filePath doesn't exist
- `PARSE_ERROR`: Invalid JSON
- `VALIDATION_FAILED`: Profile data invalid

**Side Effects**:
- Imported profile gets new createdAt timestamp
- Credentials must be re-entered by user

---

### validateProfile(profile)
**Purpose**: Validates profile data against all rules

**Preconditions**: None

**Postconditions**:
- Returns ValidationResult with:
  - `isValid`: true if all validations pass
  - `errors`: Critical validation failures
  - `warnings`: Non-critical issues

**Errors**: None (validation result returned)

**Validation Rules**:
- name: Non-empty, max 255 chars, valid filename characters
- hostname: Valid DNS or IP format
- port: 1-65535
- username: Non-empty, valid Unix username
- authMethod: Valid enum value
- sshKeyPath: File exists if authMethod = SSH_KEY
- scrollbackLines: 100-100000
- folder: Valid directory name characters (if present)

---

## File Format

### Profile JSON Structure
```json
{
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "version": "1.0",
  "name": "Production Server",
  "hostname": "prod.example.com",
  "port": 22,
  "username": "admin",
  "authMethod": "SSH_KEY",
  "sshKeyPath": "/home/user/.ssh/id_rsa",
  "folder": "Production",
  "autoReconnect": true,
  "scrollbackLines": 10000,
  "createdAt": "2025-10-05T10:30:00Z",
  "lastModified": "2025-10-05T10:30:00Z",
  "lastUsed": "2025-10-05T14:22:00Z"
}
```

### Storage Locations

**Linux**: `~/.config/ssh-client/profiles/`
**macOS**: `~/Library/Application Support/SSHClient/profiles/`
**Windows**: `%APPDATA%\SSHClient\profiles\`

---

## Error Handling

All errors returned as `Result<T>` with descriptive messages:

```cpp
// Success case
Result<ProfileId> result = createProfile(profile);
if (result.isSuccess()) {
    ProfileId id = result.value();
}

// Error case
if (result.isError()) {
    std::string error = result.error();
    // Log or display error
}
```

---

## Thread Safety

- All methods must be thread-safe
- File locking used to prevent concurrent writes
- Cache invalidation on external file changes (optional)

---

## Performance Requirements

- `createProfile()`: < 50ms
- `readProfile()`: < 10ms
- `updateProfile()`: < 50ms
- `deleteProfile()`: < 50ms
- `listProfiles()`: < 100ms for 1000 profiles
- `searchProfiles()`: < 50ms for 1000 profiles

---

## Test Scenarios

1. **Create profile**: createProfile() → profile file exists
2. **Read profile**: readProfile() → correct data returned
3. **Update profile**: updateProfile() → file contents updated
4. **Delete profile**: deleteProfile() → file removed
5. **List all**: listProfiles() → all profiles returned
6. **List by folder**: listProfilesInFolder("prod") → filtered list
7. **Search**: searchProfiles("server") → matching profiles
8. **Export**: exportProfile() → external file created
9. **Import**: importProfile() → new profile added
10. **Validation**: validateProfile(invalid) → errors reported
11. **Duplicate name**: createProfile(duplicate name) → DUPLICATE_NAME error
12. **Missing file**: readProfile(nonexistent) → PROFILE_NOT_FOUND error
13. **Corrupted file**: readProfile(corrupt JSON) → PARSE_ERROR error

---

**Status**: Contract defined, ready for test generation

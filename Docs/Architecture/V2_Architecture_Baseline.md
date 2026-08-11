# Enhance Vocabulary V2 Architecture Baseline

**Jira story:** EV-1690 - Audit V1.2 Architecture Before V2.0  
**Audit scope:** repository state on branch `story-ev-1690-audit-v1.2-architecture-before-v2.0`  
**Engine:** Unreal Engine 5.7  
**Audit type:** static, read-only source review  

## Purpose and authority

This document records the V1.2 architecture that exists before V2 work begins. It covers the areas requested by EV-1690 and its subtasks:

- Unreal modules and dependencies
- local vocabulary schema and storage
- Web and provider architecture
- V1.2 translation-language architecture
- English-USA database-context foundation
- Open/Background/Closed persistence
- import/export
- notifications
- UI ownership and Player Controller integration
- V2 blockers and technical debt

This is an analysis artifact, not an implementation plan or authorization to modify code. Nothing identified here should be fixed, refactored, moved, renamed, or reconfigured under EV-1690. Each accepted change requires separately agreed scope and, where appropriate, its own Jira work.

## Audit method and limitations

The audit reviewed project and target declarations, all module rule files, canonical and legacy vocabulary types, language mappings, normalized SQLite schema and migrations, storage CRUD and exchange paths, provider registry/URL/parsing/search paths, GameInstance lifecycle, Android device/notification integration, Player Controller orchestration, and representative UI consumers.

No build, Play In Editor session, Android package, database migration run, network request, import/export operation, or device notification test was performed. The repository has no Unreal Automation Test suite under `Source`. Runtime claims in this document are therefore source-derived and must be validated by future targeted tests before architectural changes are accepted.

## Executive baseline

V1.2 has a meaningful V2 foundation already in place:

- six explicit runtime modules;
- a normalized schema-version-3 vocabulary model;
- transactional record writes and database imports;
- a canonical `FEVVocabularyRecord` aggregate;
- separate database-context and translation-language concepts;
- language-code mapping and persisted preferences;
- provider DTO/parser separation;
- platform file-exchange abstraction;
- Android-native notification scheduling that can operate while Unreal is backgrounded or closed;
- Core interfaces that allow the Player Controller to address the root UI without including concrete UI classes.

The architecture is nevertheless transitional rather than V2-ready. Canonical normalized records coexist with flat V1 compatibility models, import/export still encodes only Russian and Ukrainian translations, provider composition is switch-based and single-request-stateful, UI code depends on the application module, and lifecycle/device behavior lacks automated verification. The highest-risk V2 work is not adding new features; it is removing or isolating these compatibility seams without data loss or behavior regression.

## 1. Unreal modules and dependencies (EV-1691)

### Current modules

| Module | Current responsibility |
|---|---|
| `EnhanceVocabularyCore` | Canonical vocabulary/language types, cross-module interfaces, validation, JSON helpers, shared action/status types |
| `EnhanceVocabularyStorage` | SQLite schema, migrations, normalized CRUD, compatibility flattening, CSV import/export and validation |
| `EnhanceVocabularyWeb` | HTTP/connectivity, provider metadata and URLs, provider DTOs/parsers, word-search aggregation |
| `EnhanceVocabularyDevice` | Platform file exchange, Android JNI/UPL, alarms, notifications, permission/settings bridges |
| `EnhanceVocabularyUI` | UMG widget classes, presentation filtering, settings controls, review/add/import-export views |
| `EnhanceVocabulary` | GameInstance service composition, Player Controller orchestration, game mode and application startup |

All six modules are declared as runtime modules in `EnhanceVocabulary.uproject` and are added explicitly to both game and editor targets.

### Current feature dependency direction

```text
EnhanceVocabularyCore
    ^
    |-- EnhanceVocabularyWeb
    |-- EnhanceVocabularyStorage
    |-- EnhanceVocabularyDevice

EnhanceVocabulary
    |-- EnhanceVocabularyCore
    |-- EnhanceVocabularyWeb
    |-- EnhanceVocabularyStorage
    `-- EnhanceVocabularyDevice

EnhanceVocabularyUI
    |-- EnhanceVocabulary
    |-- EnhanceVocabularyCore
    |-- EnhanceVocabularyWeb
    `-- EnhanceVocabularyStorage
```

### Findings

1. Core is the intended dependency floor and does not depend on the feature modules.
2. The UI module depends on the application module because widgets directly cast to `UEVGameInstance`. This makes UI reuse and isolated UI testing harder and creates a high-level-to-high-level coupling seam.
3. The application module avoids a compile-time dependency on the concrete UI module by creating generic `UUserWidget` classes and communicating through Core interfaces. This is a useful boundary, but it coexists with the reverse UI-to-application dependency.
4. Module rules expose many engine and feature dependencies in both public and private lists. `EnhanceVocabularyUI.Build.cs` also repeats `EnhanceVocabularyStorage`. The broad public surfaces increase rebuild/coupling cost and obscure which headers truly require each dependency.
5. Core publicly exposes JSON/plugin dependencies and InputCore. That may be justified by current public headers, but the dependency surface should be measured before V2 boundary work rather than assumed permanent.
6. There are no module-specific test targets or automation suites.

### V2 implication

Preserve the six-module ownership model initially. Before changing module topology, define an application-facing service/interface boundary that UI can consume without depending directly on `EnhanceVocabulary`. Dependency-list cleanup should be separate mechanical work backed by a successful editor build.

## 2. Local vocabulary schema and storage (EV-1692)

### Canonical model

`FEVVocabularyRecord` is the canonical aggregate:

```text
VocabularyRecord
    |-- Pronunciations[]
    |-- Meanings[]
    |     |-- Definitions[]
    |     |-- Translations[]
    |     `-- Relations[]
    `-- GeneralTranslations[]
```

The storage schema is version 3 and contains:

- `VocabularyEntries`
- `VocabularyPronunciations`
- `VocabularyMeanings`
- `VocabularyDefinitions`
- `VocabularyTranslations`
- `VocabularyRelations`

Foreign keys use `ON DELETE CASCADE`; root identity is protected by a unique normalized-word index. Display ordering, provider provenance, confidence, timestamps, and reserved metadata columns are represented.

### Persistence behavior

- Storage opens a context-specific SQLite database with `ReadWriteCreate` and enables foreign keys.
- Fresh schema creation is descriptor-driven.
- Legacy flat-table migration renames the table, creates normalized tables, converts each row, and drops the legacy table inside an immediate transaction.
- The version-2-to-version-3 migration adds pronunciation `LanguageCode` transactionally.
- Canonical record insert and replace operations are transactional when invoked through public APIs.
- Replace keeps the root row, updates its presentation word/timestamp, deletes children, and reinserts the complete child graph.
- Bulk append and overwrite run inside one transaction and roll back on failure.

### Strengths

- Canonical normalized storage matches the richer domain aggregate.
- Migration and write paths use prepared bindings for values.
- Flat compatibility is isolated behind conversion/flattening helpers rather than duplicated across every caller.
- Database-context paths preserve the existing English-USA filename, protecting current users from an accidental apparent data loss.

### Transitional seams and debt

1. `FVocabularyEntry` and `FWordSearchResult` retain flat V1 fields. Review, controller, compatibility queries, notifications, and exchange code still depend on those adapters.
2. Compatibility projections support only one transcription plus flattened definitions/usages and Russian/Ukrainian translations. They cannot represent the full normalized aggregate without information loss.
3. Child tables do not declare uniqueness constraints for logical duplicates. De-duplication is mostly application-side and can vary by ingestion path.
4. Schema `MetadataJson` columns are present but not populated by current insert paths.
5. Migration logic is a small set of special cases rather than an explicit ordered migration registry. Future versions will need a disciplined version-to-version chain and fixtures.
6. Fresh schema creation executes multiple DDL statements and sets the version afterward without wrapping the whole sequence in one explicit transaction.
7. Editor storage points the English-USA debug database into `Content/VocabularyDB/DebugDBFile`. Running editor workflows can therefore mutate a project-content location, which is risky for source-control hygiene and repeatable tests.
8. Export and notification record loading perform a root enumeration followed by per-record reconstruction. This is acceptable at small scale but becomes an N+1-style performance risk as vocabulary size grows.
9. No automated tests prove legacy migration, version-3 migration, rollback behavior, Unicode preservation, ordering, cascade deletes, or round-trip equivalence.

## 3. Web and provider architecture (EV-1693)

### Current flow

```text
UI/provider selection
    -> GameInstance
    -> UEVWordSearchService
        |-- provider registry
        |-- URL builder
        |-- shared HTTP service
        |-- provider DTO/parser
        `-- canonical FEVVocabularyRecord + legacy result projection
```

FreeDictionary provides dictionary/phonetic/meaning/relation data. MyMemory provides translations. Datamuse is registered as metadata but is not handled by the URL builder and is not exposed by the current UI provider controls.

### Strengths

- Provider response DTOs are separated from canonical domain types.
- Parsing produces canonical records rather than leaking raw DTOs into storage/UI.
- Dictionary and translation requests run independently and are merged.
- Translation targets are normalized, de-duplicated, and filtered against the active database context.
- Provider provenance and confidence can be stored.

### Blockers and debt

1. `UEVWordSearchService` owns one mutable pending result, record, word, dictionary state, and translation counter. Overlapping searches can mix callbacks from different requests. V2 must either prohibit concurrency explicitly or introduce per-search request state/identity and cancellation.
2. Provider extensibility is switch-based across the enum, registry, URL builder, parser selection, and UI. It is a catalog, not a polymorphic provider boundary.
3. Datamuse is advertised by the registry but unsupported in URL construction and ignored by the UI. Registry availability and operational capability can diverge.
4. URLs concatenate the word and query values directly without an explicit URL-encoding step.
5. `ProcessRequest()` results and request handles are not retained for cancellation or lifecycle management.
6. The HTTP service always invokes the FreeDictionary debug-response routine. For matching responses it logs the complete body and writes it to `Saved/Debug`, despite the comment implying opt-in behavior. This is production-noise, storage, and potential data-handling debt.
7. Connectivity is inferred through a periodic request to a Google `generate_204` endpoint. This couples application online state to an unrelated third party and does not prove that configured vocabulary providers are reachable.
8. Provider behavior has no contract tests, fixtures, retry/backoff policy, response-size limits, or schema-change detection.

## 4. V1.2 translation-language architecture (EV-1694)

### Current foundation

Database context and translation language are deliberately separate enums:

- active database context: English USA only;
- selectable translations: English USA, English UK, Russian, Ukrainian, Spanish, German, French, Italian.

`FEVVocabularyLanguagePreferences` persists the selected context and translation list. Normalization removes `None`, duplicates, and a translation equivalent to the active database context. Stable storage and web codes are mapped centrally. Word search issues one translation request per selected target. UI display code filters canonical translations by selected language codes.

### Strengths

- The design correctly distinguishes source vocabulary identity from translation targets.
- Preferences support zero selected translations as a valid state.
- Ukrainian legacy `ua` and broad English `en` aliases are handled at compatibility edges.
- Translation rows are atomic rather than comma-packed in canonical storage.

### Blockers and debt

1. The canonical path is multi-language, but flat compatibility models and CSV exchange expose only `TranslationRu` and `TranslationUa`. Other selected languages can be fetched and stored yet are omitted from compatibility reads and exports.
2. Legacy UI/controller paths still materialize `FVocabularyEntry`, making language completeness dependent on which UI path is used.
3. Translation filtering/mapping logic is repeated in multiple widgets.
4. Target language is stored as free text in SQLite. Central mapping helps current writes, but database-level validation and a migration policy for renamed codes do not exist.
5. MyMemory translations generally remain entry-level because the provider cannot reliably identify a source meaning. Meaning-aware translation editing and merging need an explicit V2 rule.
6. There is no documented conflict policy for results from multiple providers, confidence precedence, manual edits, or translation deduplication across entry-level and meaning-level values.

## 5. English-USA database-context foundation (EV-1695)

### Current behavior

- `EEVVocabularyDBContext` currently exposes only `EnglishUSA` plus `None`.
- English USA maps to stable ID `en-US`, Web source code `en`, and pronunciation code `en`.
- Preferences normalize `None` back to English USA.
- Editor and packaged paths are selected per context; English USA deliberately retains legacy filenames.
- Changing context shuts down storage, opens the new context database, and restores the previous context on failure.
- Controller methods include intentionally empty future entry points for creating a context and cross-context Review/Add flows.

### Assessment

This is a sound compatibility foundation, not a completed multi-context architecture. The enum and helper switches provide a clear replacement point, and the per-context path policy avoids mixing future vocabularies. However, adding a second context will require coordinated work in context discovery, file naming, provider capability, pronunciation mapping, UI navigation, notification payload ownership, import/export context metadata, migrations, and rollback UX.

### V2 blockers

1. Context availability is compiled into an enum/static list rather than data-driven configuration.
2. Unsupported contexts map to empty IDs/codes; extension requires every helper switch to be updated consistently.
3. CSV files carry no database-context identity. Importing a file into the wrong context cannot be detected from the format.
4. Notification payloads are snapshots from the active database at schedule time; behavior when changing context while a schedule exists is not explicitly defined.
5. The editor database path for English USA remains inside Content and should be addressed under separate, migration-aware work.

## 6. Open/Background/Closed persistence (EV-1696)

### Open

`UEVGameInstance::Init` loads language preferences, creates Web/connectivity/storage/device services, opens the context database, starts connectivity polling, and binds device callbacks. The Player Controller creates the root UI and binds Core-defined UI events to orchestration handlers.

### Background

The GameInstance subscribes to `ApplicationWillEnterBackground` and flushes language preferences to `GGameUserSettingsIni`. SQLite remains open; individual database mutations have already been committed synchronously through their operation-level transactions. Android notification schedules and payloads live in native `AlarmManager`/`SharedPreferences`, outside Unreal timers.

### Closed

On orderly Unreal shutdown, preferences are flushed, SQLite is closed, polling stops, and connectivity shuts down. If the process is killed without orderly shutdown, committed SQLite transactions and previously applied Android shared preferences are the durability boundary. Android `BroadcastReceiver` alarms can post notifications without a running Unreal process.

### Gaps and debt

1. There is no lifecycle test matrix for foreground, background, force-stop, process eviction, activity recreation, app upgrade, device reboot, or database-in-use cases.
2. No explicit database checkpoint/close occurs on background. This is not automatically incorrect, but the intended durability and file-exchange interaction must be verified.
3. Language preferences are persisted in Unreal config, notification settings in Android shared preferences, and vocabulary in SQLite. There is no single documented consistency boundary across the three stores.
4. No boot receiver/reschedule path is present, so schedules are not demonstrated to survive device reboot.
5. Notification payloads are serialized snapshots. Adding, editing, deleting, importing, or switching context after scheduling does not visibly refresh the native payload automatically.
6. Force-stop behavior is controlled by Android and is not established by static review.

## 7. Import/export (EV-1697)

### Current design

The UI emits operation requests; the Player Controller owns confirmations, spinners, and completion presentation; GameInstance orchestrates storage and device services; Storage validates/converts data; Device selects platform-specific file exchange.

Supported operations are CSV template download, database export, overwrite import, append import, and validation-report export. The exchange schema is flat but supports repeated rows per word to reconstruct normalized meanings. It includes word, part of speech, transcription, audio URL, definition, usage, Russian/Ukrainian translations, synonyms, and antonyms.

### Strengths

- Destructive overwrite and append flows require UI confirmation.
- CSV headers are validated and normalized.
- Input words are normalized through the shared validator.
- Duplicate source rows are grouped into canonical records.
- Append conflicts are checked before database mutation.
- Invalid rows/conflicts produce downloadable validation reports.
- Append and overwrite are transactional.
- Platform save/load behavior is behind an interface with editor, Android, and unsupported implementations.

### Blockers and debt

1. Export is not language-complete: only Russian and Ukrainian translations round-trip. Spanish, German, French, Italian, English UK, provider provenance, confidence, pronunciation metadata, licenses, and metadata JSON are lost.
2. The format has no schema version, application version, database-context ID, or locale declaration.
3. Import assumes pronunciation language `en` and only recognizes RU/UK translation columns.
4. Flat repeated-row reconstruction has ambiguity around meaning identity, definition/usage pairing, and general versus meaning-level translations. Current conventions are encoded in parser behavior rather than a versioned external contract.
5. Export reconstructs each full record individually, which may scale poorly.
6. There are no golden CSV fixtures or automated round-trip, Unicode, quoting, malformed-row, rollback, or cross-version tests.
7. Import/export orchestration keeps mutable pending-operation state across asynchronous platform callbacks; explicit request identity would reduce accidental cross-operation state mixing.

## 8. Notifications (EV-1698)

### Current architecture

- UI edits interval/mode.
- Player Controller owns confirmation, permission, pending-transition, accepted-settings, foreground sync, and navigation behavior.
- GameInstance builds notification payloads from storage and selects/schedules the appropriate device path.
- DeviceService bridges C++ to Android.
- Android `EVVocabularyAlarmReceiver` stores schedule state/payload in `SharedPreferences`, schedules one alarm, posts a notification, and schedules the next alarm.
- A notification tap carries word details back to `GameActivity`; C++ consumes the pending word and asks the root UI to open Review Words.

Random-word selection avoids immediately repeating the last word/meaning/definition when alternatives exist. Android 13+ notification permission and app notification settings are integrated.

### Confirmed blocker

`UEVGameInstance` schedules TestMode with an empty payload, but `UEVDeviceService::ScheduleVocabularyNotifications` rejects empty payloads before reaching Android, and the Java scheduler also rejects empty serialized words. TestMode therefore cannot use the normal schedule path as written.

### Additional debt

1. Native and C++ selection logic both encode notification behavior, creating two implementations to keep aligned.
2. Schedule payloads are snapshots and are not automatically refreshed after vocabulary mutations or context changes.
3. Reboot persistence is not implemented/demonstrated.
4. Exact timing is not guaranteed: Android uses `setAndAllowWhileIdle`/`set`, and OS power policy can defer delivery.
5. TestMode has no completed notification action even in the receiver; it logs when fired.
6. Notification state is split between controller transient state and native persistent state, requiring foreground reconciliation.
7. The UPL test payload contains visibly corrupted phonetic text in source, indicating an encoding hygiene issue in embedded Java/XML content.
8. There are no device tests covering permission denial, settings disablement, tap routing for warm/cold app, cancel action, process death, or long-running rescheduling.

## 9. UI ownership and Player Controller integration (EV-1699)

### Current ownership

The root UMG widget owns screen composition and emits events through Core interfaces. The Player Controller creates root/overlay widgets and orchestrates:

- UI event binding;
- confirmations;
- loading and action-status overlays;
- detailed-entry editing/deletion;
- import/export flow;
- notification permission/settings transitions;
- notification-tap navigation;
- language-preference application;
- relation/translation actions and filter overlays.

GameInstance owns long-lived services and application data operations. UI widgets also directly query/cast GameInstance for preferences and record operations in several paths.

### Strengths

- Root UI communication uses Core interfaces/delegates, avoiding a concrete UI dependency from the application module.
- Destructive or mode-changing operations are mediated by controller confirmation state.
- Widgets generally own presentation/local edit state while controller/GameInstance own commits.

### Debt

1. `AEVAppPlayerController` is a large coordinator with many unrelated state machines. Notification, import/export, vocabulary editing, overlay management, filtering, and navigation compete in one class.
2. UI widgets directly depend on `UEVGameInstance`, so the conceptual interface boundary is incomplete.
3. Numerous optional interface/delegate bindings are established manually in `BeginPlay`; missing Blueprint bindings are detected only at runtime through logs.
4. Mutable pending state exists for confirmations, file operations, notification transitions, value actions, and widget overlays. Without request identity, unrelated asynchronous completions can be hard to reason about.
5. Legacy flat entries still cross UI/controller boundaries even where canonical records exist.
6. UI behavior has no automated tests, and correctness depends on Blueprint class assignment and binding names not visible in C++ static review.

## 10. Ranked V2 blockers and technical debt (EV-1700)

### Blockers - resolve or explicitly accept before dependent V2 work

| ID | Finding | Why it blocks V2 |
|---|---|---|
| B1 | No automated schema migration, storage round-trip, import/export, provider, lifecycle, or UI tests | V2 changes can silently lose user data or regress Android behavior |
| B2 | Canonical records coexist with flat `FVocabularyEntry`/legacy search projections | New features may be implemented against incompatible models and lose normalized data |
| B3 | CSV exchange supports only RU/UK translations and has no context/schema identity | Multi-language/context data cannot safely round-trip or be validated |
| B4 | Word-search service has one mutable pending-search state | Concurrent/overlapping searches can mix provider callbacks and results |
| B5 | Provider architecture is enum/switch based; registry capability can be false | Adding/replacing providers requires broad coordinated edits and weakens capability discovery |
| B6 | TestMode scheduling rejects its own empty payload | The advertised notification mode is not viable through the normal path |
| B7 | Notification schedule/payload refresh rules are undefined across edits, imports, deletes, and context changes | Closed-app notifications can use stale vocabulary or the wrong context |
| B8 | UI depends directly on the application module while the controller is an oversized coordinator | V2 UI/application changes will amplify coupling and make isolated verification difficult |
| B9 | English-USA editor database resides under project Content | Editor use can mutate project content and complicate safe migration/test fixtures |
| B10 | No verified lifecycle matrix, including reboot/process death/force-stop | Persistence claims needed by V2 cannot be accepted from static code alone |

### Significant technical debt

| ID | Finding |
|---|---|
| D1 | Broad/duplicated public and private module dependencies |
| D2 | Application-side duplicate control without child-table uniqueness constraints |
| D3 | Special-case migrations rather than a versioned migration registry with fixtures |
| D4 | N+1-style complete-record reconstruction for exports and notification payloads |
| D5 | Full FreeDictionary response logging/file output enabled in the shared HTTP path |
| D6 | Google-specific connectivity probe rather than provider-aware health |
| D7 | URL values are concatenated without an explicit encoding boundary |
| D8 | Language filtering/mapping compatibility logic is repeated across widgets |
| D9 | Unused/reserved metadata columns without a defined ownership/serialization policy |
| D10 | Async file/notification/search flows rely on shared mutable pending state rather than operation IDs |
| D11 | Android embedded-source encoding corruption is visible in test/notification strings |
| D12 | No reboot rescheduling path for persisted notification preferences |

## Recommended follow-up sequence

This sequence is advisory only; it does not authorize implementation.

1. **Establish characterization tests and fixtures.** Capture schema-v1-to-v3 migrations, canonical CRUD, CSV round trips, provider parsing fixtures, language preference normalization, and Android lifecycle scenarios.
2. **Define V2 canonical contracts.** Decide whether `FEVVocabularyRecord` is the sole application model, define translation/meaning identity and merge rules, and version the external exchange contract.
3. **Protect data migration.** Move editor/runtime database policy and implement ordered migrations with backups/fixtures before removing compatibility code.
4. **Make Web operations request-scoped.** Introduce request identity, cancellation, provider capability contracts, encoding, and deterministic merge/error behavior.
5. **Generalize import/export.** Include context/schema metadata and lossless dynamic-language representation before enabling additional production contexts.
6. **Define notification consistency.** Specify refresh triggers, context switching, reboot behavior, TestMode semantics, and the source of truth shared by controller/native layers.
7. **Reduce UI/application coupling.** Introduce narrow application-facing interfaces or coordinators, then split Player Controller state machines incrementally with characterization coverage.
8. **Only then remove legacy projections.** Retire flat models path by path after all consumers use canonical records and migrations/export compatibility are proven.

## Jira subtask completion map

| Jira | Audit evidence in this document |
|---|---|
| EV-1691 | Section 1 - modules/dependencies |
| EV-1692 | Section 2 - schema/storage |
| EV-1693 | Section 3 - Web/providers |
| EV-1694 | Section 4 - translation languages |
| EV-1695 | Section 5 - English-USA DB context |
| EV-1696 | Section 6 - Open/Background/Closed persistence |
| EV-1697 | Section 7 - import/export |
| EV-1698 | Section 8 - notifications |
| EV-1699 | Section 9 - UI/Player Controller |
| EV-1700 | Section 10 - blockers/debt |
| EV-1701 | This baseline artifact |

## Completion statement

EV-1690's static architecture-audit scope is covered by this baseline. Completion of the audit does not mean the architecture is approved for V2 implementation, nor that the identified blockers have been resolved. The next action is human review and creation/prioritization of separately scoped Jira work. No code, asset, configuration, schema, module, or Jira-state change is authorized by this document.

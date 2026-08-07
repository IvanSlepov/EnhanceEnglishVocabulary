package com.epicgames.unreal;

import android.Manifest;
import android.app.AlarmManager;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import org.json.JSONArray;
import org.json.JSONObject;

public class EVVocabularyAlarmReceiver extends BroadcastReceiver
{
    public static final String ACTION_FIRE =
        "com.epicgames.unreal.EV_VOCABULARY_ALARM_FIRE";

    public static final String ACTION_CANCEL =
        "com.epicgames.unreal.EV_VOCABULARY_ALARM_CANCEL";

    private static final String CHANNEL_ID = "ev_vocabulary_reminders";
    private static final int NOTIFICATION_ID = 7353;
    private static final int ALARM_REQUEST_CODE = 7355;
    private static final int CANCEL_REQUEST_CODE = 7356;

    private static final String PREFERENCES = "EVNotificationPreferences";
    private static final String KEY_ENABLED = "VocabularyAlarmEnabled";
    private static final String KEY_INTERVAL_MILLISECONDS = "VocabularyAlarmIntervalMilliseconds";
    private static final String KEY_WORDS = "VocabularyAlarmWords";
    private static final String KEY_MODE = "VocabularyAlarmMode";
    private static final String KEY_LAST_NORMALIZED_WORD = "VocabularyAlarmLastNormalizedWord";
    private static final String KEY_LAST_MEANING_DISPLAY_ORDER = "VocabularyAlarmLastMeaningDisplayOrder";
    private static final String KEY_LAST_DEFINITION_DISPLAY_ORDER = "VocabularyAlarmLastDefinitionDisplayOrder";
    public static final String EXTRA_NOTIFICATION_WORD =
        "com.epicgames.unreal.EV_NOTIFICATION_WORD";
    public static final String EXTRA_NOTIFICATION_NORMALIZED_WORD =
        "com.epicgames.unreal.EV_NOTIFICATION_NORMALIZED_WORD";
    public static final String EXTRA_NOTIFICATION_TRANSCRIPTION =
        "com.epicgames.unreal.EV_NOTIFICATION_TRANSCRIPTION";
    public static final String EXTRA_NOTIFICATION_PART_OF_SPEECH =
        "com.epicgames.unreal.EV_NOTIFICATION_PART_OF_SPEECH";
    public static final String EXTRA_NOTIFICATION_MEANING_DISPLAY_ORDER =
        "com.epicgames.unreal.EV_NOTIFICATION_MEANING_DISPLAY_ORDER";
    public static final String EXTRA_NOTIFICATION_DEFINITION_TEXT =
        "com.epicgames.unreal.EV_NOTIFICATION_DEFINITION_TEXT";
    public static final String EXTRA_NOTIFICATION_DEFINITION_DISPLAY_ORDER =
        "com.epicgames.unreal.EV_NOTIFICATION_DEFINITION_DISPLAY_ORDER";
    public static final String EXTRA_NOTIFICATION_MODE =
        "com.epicgames.unreal.EV_NOTIFICATION_MODE";

    @Override
    public void onReceive(Context context, Intent intent)
    {
        final String action = intent != null ? intent.getAction() : null;

        if (ACTION_CANCEL.equals(action))
        {
            cancel(context);
            return;
        }

        SharedPreferences preferences =
            context.getSharedPreferences(PREFERENCES, Context.MODE_PRIVATE);

        if (!preferences.getBoolean(KEY_ENABLED, false))
        {
            Log.i("EVAlarm", "Ignoring disabled vocabulary alarm.");
            return;
        }

        try
        {
            final int notificationMode = preferences.getInt(KEY_MODE, 0);

            switch (notificationMode)
            {
                case 0: // EEVNotificationMode::RandomWord
                    showNotification(
                        context,
                        preferences,
                        preferences.getString(KEY_WORDS, ""));
                    break;

                case 1: // EEVNotificationMode::TestMode
                    Log.i("EVAlarm", "TestMode alarm fired. No notification action is assigned.");
                    break;

                default:
                    Log.w("EVAlarm", "Unsupported notification mode: " + notificationMode);
                    break;
            }
        }
        catch (Exception exception)
        {
            Log.e("EVAlarm", "Failed to show vocabulary notification.", exception);
        }
        finally
        {
            long intervalMilliseconds =
                preferences.getLong(KEY_INTERVAL_MILLISECONDS, 0L);

            if (intervalMilliseconds > 0L &&
                preferences.getBoolean(KEY_ENABLED, false))
            {
                scheduleNext(context, intervalMilliseconds);
            }
        }
    }

    public static boolean schedule(
        Context context,
        int intervalSeconds,
        String serializedWords,
        int notificationMode)
    {
        if (context == null || intervalSeconds <= 0 ||
            serializedWords == null || serializedWords.trim().isEmpty())
        {
            return false;
        }

        long intervalMilliseconds = intervalSeconds * 1000L;

        context.getSharedPreferences(PREFERENCES, Context.MODE_PRIVATE)
            .edit()
            .putBoolean(KEY_ENABLED, true)
            .putLong(KEY_INTERVAL_MILLISECONDS, intervalMilliseconds)
            .putString(KEY_WORDS, serializedWords)
            .putInt(KEY_MODE, notificationMode)
            .apply();

        return scheduleNext(context, intervalMilliseconds);
    }

    public static boolean cancel(Context context)
    {
        if (context == null)
        {
            return false;
        }

        context.getSharedPreferences(PREFERENCES, Context.MODE_PRIVATE)
            .edit()
            .putBoolean(KEY_ENABLED, false)
            .remove(KEY_INTERVAL_MILLISECONDS)
            .remove(KEY_WORDS)
            .remove(KEY_LAST_NORMALIZED_WORD)
            .remove(KEY_LAST_MEANING_DISPLAY_ORDER)
            .remove(KEY_LAST_DEFINITION_DISPLAY_ORDER)
            .apply();

        AlarmManager alarmManager =
            (AlarmManager)context.getSystemService(Context.ALARM_SERVICE);

        if (alarmManager != null)
        {
            alarmManager.cancel(createAlarmPendingIntent(context));
        }

        NotificationManager notificationManager =
            (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);

        if (notificationManager != null)
        {
            notificationManager.cancel(NOTIFICATION_ID);
        }

        Log.i("EVAlarm", "Vocabulary notifications cancelled.");
        return true;
    }

    private static boolean scheduleNext(Context context, long intervalMilliseconds)
    {
        AlarmManager alarmManager =
            (AlarmManager)context.getSystemService(Context.ALARM_SERVICE);

        if (alarmManager == null)
        {
            Log.e("EVAlarm", "AlarmManager is null.");
            return false;
        }

        long triggerAtMillis =
            System.currentTimeMillis() + intervalMilliseconds;

        PendingIntent pendingIntent = createAlarmPendingIntent(context);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
        {
            alarmManager.setAndAllowWhileIdle(
                AlarmManager.RTC_WAKEUP,
                triggerAtMillis,
                pendingIntent);
        }
        else
        {
            alarmManager.set(
                AlarmManager.RTC_WAKEUP,
                triggerAtMillis,
                pendingIntent);
        }

        Log.i("EVAlarm", "Vocabulary alarm scheduled for " + triggerAtMillis);
        return true;
    }

    private static PendingIntent createAlarmPendingIntent(Context context)
    {
        Intent intent = new Intent(context, EVVocabularyAlarmReceiver.class);
        intent.setAction(ACTION_FIRE);

        return PendingIntent.getBroadcast(
            context,
            ALARM_REQUEST_CODE,
            intent,
            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
    }

    private static PendingIntent createCancelPendingIntent(Context context)
    {
        Intent intent = new Intent(context, EVVocabularyAlarmReceiver.class);
        intent.setAction(ACTION_CANCEL);

        return PendingIntent.getBroadcast(
            context,
            CANCEL_REQUEST_CODE,
            intent,
            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
    }

    private static void showNotification(
        Context context,
        SharedPreferences preferences,
        String serializedPayload)
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU &&
            context.checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS)
                != PackageManager.PERMISSION_GRANTED)
        {
            Log.w("EVAlarm", "Notification permission is not granted.");
            return;
        }

        NotificationSelection selection =
            selectRandomNotification(
                serializedPayload,
                preferences.getString(KEY_LAST_NORMALIZED_WORD, ""),
                preferences.getInt(KEY_LAST_MEANING_DISPLAY_ORDER, Integer.MIN_VALUE),
                preferences.getInt(KEY_LAST_DEFINITION_DISPLAY_ORDER, Integer.MIN_VALUE));

        if (selection == null)
        {
            Log.w("EVAlarm", "No vocabulary definition is available for the notification.");
            return;
        }

        NotificationManager notificationManager =
            (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);

        if (notificationManager == null)
        {
            Log.e("EVAlarm", "NotificationManager is null.");
            return;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
        {
            NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID,
                "Vocabulary reminders",
                NotificationManager.IMPORTANCE_HIGH);

            channel.setDescription("Periodic vocabulary word reminders");
            notificationManager.createNotificationChannel(channel);
        }

        Intent openWordIntent = new Intent(context, GameActivity.class)
            .putExtra(EXTRA_NOTIFICATION_WORD, selection.word)
            .putExtra(EXTRA_NOTIFICATION_NORMALIZED_WORD, selection.normalizedWord)
            .putExtra(EXTRA_NOTIFICATION_TRANSCRIPTION, selection.transcription)
            .putExtra(EXTRA_NOTIFICATION_PART_OF_SPEECH, selection.partOfSpeech)
            .putExtra(EXTRA_NOTIFICATION_MEANING_DISPLAY_ORDER, selection.meaningDisplayOrder)
            .putExtra(EXTRA_NOTIFICATION_DEFINITION_TEXT, selection.definitionText)
            .putExtra(EXTRA_NOTIFICATION_DEFINITION_DISPLAY_ORDER, selection.definitionDisplayOrder)
            .putExtra(EXTRA_NOTIFICATION_MODE, 0)
            .addFlags(
                Intent.FLAG_ACTIVITY_CLEAR_TOP |
                Intent.FLAG_ACTIVITY_SINGLE_TOP);

        PendingIntent contentIntent = PendingIntent.getActivity(
            context,
            NOTIFICATION_ID,
            openWordIntent,
            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

        Notification.Builder builder =
            Build.VERSION.SDK_INT >= Build.VERSION_CODES.O
                ? new Notification.Builder(context, CHANNEL_ID)
                : new Notification.Builder(context);

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O)
        {
            builder.setPriority(Notification.PRIORITY_HIGH);
        }

        String numberedDefinition =
            (selection.definitionDisplayOrder + 1) + ". " + selection.definitionText;

        String displayedTranscription =
            selection.transcription != null && !selection.transcription.trim().isEmpty()
                ? selection.transcription.trim()
                : "No transcription was provided";

        String body =
            displayedTranscription + "\n\n" +
            selection.partOfSpeech + "\n\n" +
            numberedDefinition;

        builder
            .setSmallIcon(context.getApplicationInfo().icon)
            .setContentTitle(selection.word)
            .setContentText(displayedTranscription + " — " + selection.partOfSpeech)
            .setStyle(new Notification.BigTextStyle().bigText(body))
            .setCategory(Notification.CATEGORY_REMINDER)
            .setAutoCancel(true)
            .addAction(
                0,
                "Turn Off",
                createCancelPendingIntent(context));

        if (contentIntent != null)
        {
            builder.setContentIntent(contentIntent);
        }

        preferences.edit()
            .putString(KEY_LAST_NORMALIZED_WORD, selection.normalizedWord)
            .putInt(KEY_LAST_MEANING_DISPLAY_ORDER, selection.meaningDisplayOrder)
            .putInt(KEY_LAST_DEFINITION_DISPLAY_ORDER, selection.definitionDisplayOrder)
            .apply();

        notificationManager.notify(NOTIFICATION_ID, builder.build());
        Log.i(
            "EVAlarm",
            "Vocabulary notification posted: " + selection.word +
            " | " + selection.partOfSpeech +
            " | " + numberedDefinition);
    }

    private static NotificationSelection selectRandomNotification(
        String serializedPayload,
        String lastNormalizedWord,
        int lastMeaningDisplayOrder,
        int lastDefinitionDisplayOrder)
    {
        if (serializedPayload == null || serializedPayload.trim().isEmpty())
        {
            return null;
        }

        try
        {
            JSONObject root = new JSONObject(serializedPayload);
            JSONArray records = root.optJSONArray("records");

            if (records == null || records.length() == 0)
            {
                return null;
            }

            List<JSONObject> validRecords = new ArrayList<>();

            for (int recordIndex = 0; recordIndex < records.length(); ++recordIndex)
            {
                JSONObject record = records.optJSONObject(recordIndex);

                if (record == null || record.optString("word", "").trim().isEmpty())
                {
                    continue;
                }

                JSONArray meanings = record.optJSONArray("meanings");
                if (meanings != null && meanings.length() > 0)
                {
                    validRecords.add(record);
                }
            }

            if (validRecords.isEmpty())
            {
                return null;
            }

            Random random = new Random();
            List<JSONObject> recordCandidates = new ArrayList<>(validRecords);

            if (recordCandidates.size() > 1 &&
                lastNormalizedWord != null &&
                !lastNormalizedWord.trim().isEmpty())
            {
                for (int index = recordCandidates.size() - 1; index >= 0; --index)
                {
                    JSONObject candidate = recordCandidates.get(index);
                    if (candidate.optString("normalizedWord", "")
                        .equalsIgnoreCase(lastNormalizedWord.trim()))
                    {
                        recordCandidates.remove(index);
                    }
                }
            }

            JSONObject selectedRecord =
                recordCandidates.get(random.nextInt(recordCandidates.size()));

            boolean selectedSameWord = selectedRecord
                .optString("normalizedWord", "")
                .equalsIgnoreCase(lastNormalizedWord != null ? lastNormalizedWord.trim() : "");

            JSONArray meanings = selectedRecord.getJSONArray("meanings");
            List<JSONObject> validMeanings = new ArrayList<>();

            for (int meaningIndex = 0; meaningIndex < meanings.length(); ++meaningIndex)
            {
                JSONObject meaning = meanings.optJSONObject(meaningIndex);
                JSONArray definitions = meaning != null
                    ? meaning.optJSONArray("definitions")
                    : null;

                if (definitions != null && definitions.length() > 0)
                {
                    validMeanings.add(meaning);
                }
            }

            if (validMeanings.isEmpty())
            {
                return null;
            }

            List<JSONObject> meaningCandidates = new ArrayList<>(validMeanings);

            if (selectedSameWord && meaningCandidates.size() > 1)
            {
                for (int index = meaningCandidates.size() - 1; index >= 0; --index)
                {
                    if (meaningCandidates.get(index)
                        .optInt("meaningDisplayOrder", 0) == lastMeaningDisplayOrder)
                    {
                        meaningCandidates.remove(index);
                    }
                }
            }

            JSONObject selectedMeaning =
                meaningCandidates.get(random.nextInt(meaningCandidates.size()));

            boolean selectedSameMeaning = selectedSameWord &&
                selectedMeaning.optInt("meaningDisplayOrder", 0) == lastMeaningDisplayOrder;

            JSONArray definitions = selectedMeaning.getJSONArray("definitions");
            List<JSONObject> validDefinitions = new ArrayList<>();

            for (int definitionIndex = 0;
                 definitionIndex < definitions.length();
                 ++definitionIndex)
            {
                JSONObject definition = definitions.optJSONObject(definitionIndex);

                if (definition != null &&
                    !definition.optString("definitionText", "").trim().isEmpty())
                {
                    validDefinitions.add(definition);
                }
            }

            if (validDefinitions.isEmpty())
            {
                return null;
            }

            List<JSONObject> definitionCandidates = new ArrayList<>(validDefinitions);

            if (selectedSameMeaning && definitionCandidates.size() > 1)
            {
                for (int index = definitionCandidates.size() - 1; index >= 0; --index)
                {
                    if (definitionCandidates.get(index)
                        .optInt("definitionDisplayOrder", 0) == lastDefinitionDisplayOrder)
                    {
                        definitionCandidates.remove(index);
                    }
                }
            }

            JSONObject selectedDefinition =
                definitionCandidates.get(random.nextInt(definitionCandidates.size()));

            NotificationSelection selection = new NotificationSelection();
            selection.word = selectedRecord.optString("word", "").trim();
            selection.normalizedWord =
                selectedRecord.optString("normalizedWord", "").trim();
            selection.transcription =
                selectedRecord.optString("transcription", "No transcription was provided").trim();
            selection.partOfSpeech =
                selectedMeaning.optString("partOfSpeech", "unspecified").trim();
            selection.meaningDisplayOrder =
                selectedMeaning.optInt("meaningDisplayOrder", 0);
            selection.definitionText =
                selectedDefinition.optString("definitionText", "").trim();
            selection.definitionDisplayOrder =
                selectedDefinition.optInt("definitionDisplayOrder", 0);

            if (selection.partOfSpeech.isEmpty())
            {
                selection.partOfSpeech = "unspecified";
            }

            return selection;
        }
        catch (Exception exception)
        {
            Log.e("EVAlarm", "Failed to parse RandomWord notification payload.", exception);
            return null;
        }
    }

    private static final class NotificationSelection
    {
        String word = "";
        String normalizedWord = "";
        String transcription = "No transcription was provided";
        String partOfSpeech = "unspecified";
        int meaningDisplayOrder = 0;
        String definitionText = "";
        int definitionDisplayOrder = 0;
    }

}

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

public class EVVocabularyAlarmReceiver extends BroadcastReceiver
{
    public static final String ACTION_FIRE =
        "com.epicgames.unreal.EV_VOCABULARY_ALARM_FIRE";

    public static final String ACTION_CANCEL =
        "com.epicgames.unreal.EV_VOCABULARY_ALARM_CANCEL";

    public static final String ACTION_OPEN_WORD =
        "com.epicgames.unreal.EV_VOCABULARY_OPEN_WORD";

    private static final String CHANNEL_ID = "ev_vocabulary_reminders";
    private static final int NOTIFICATION_ID = 7353;
    private static final int ALARM_REQUEST_CODE = 7355;
    private static final int CANCEL_REQUEST_CODE = 7356;

    private static final String PREFERENCES = "EVNotificationPreferences";
    private static final String KEY_ENABLED = "VocabularyAlarmEnabled";
    private static final String KEY_INTERVAL_MILLISECONDS = "VocabularyAlarmIntervalMilliseconds";
    private static final String KEY_WORDS = "VocabularyAlarmWords";
    private static final String KEY_MODE = "VocabularyAlarmMode";
    private static final String KEY_PENDING_NOTIFICATION_WORD = "PendingNotificationWord";

    public static final String EXTRA_NOTIFICATION_WORD =
        "com.epicgames.unreal.EV_NOTIFICATION_WORD";

    @Override
    public void onReceive(Context context, Intent intent)
    {
        final String action = intent != null ? intent.getAction() : null;

        if (ACTION_CANCEL.equals(action))
        {
            cancel(context);
            return;
        }

        if (ACTION_OPEN_WORD.equals(action))
        {
            final String word = intent != null
                ? intent.getStringExtra(EXTRA_NOTIFICATION_WORD)
                : null;

            if (word != null && !word.isEmpty())
            {
                context.getSharedPreferences(PREFERENCES, Context.MODE_PRIVATE)
                    .edit()
                    .putString(KEY_PENDING_NOTIFICATION_WORD, word)
                    .apply();
            }

            Intent launchIntent =
                context.getPackageManager().getLaunchIntentForPackage(
                    context.getPackageName());

            if (launchIntent != null)
            {
                launchIntent.addFlags(
                    Intent.FLAG_ACTIVITY_NEW_TASK |
                    Intent.FLAG_ACTIVITY_CLEAR_TOP |
                    Intent.FLAG_ACTIVITY_SINGLE_TOP);
                context.startActivity(launchIntent);
            }

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
                    showNotification(context, preferences.getString(KEY_WORDS, ""));
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

    private static void showNotification(Context context, String serializedWords)
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU &&
            context.checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS)
                != PackageManager.PERMISSION_GRANTED)
        {
            Log.w("EVAlarm", "Notification permission is not granted.");
            return;
        }

        String word = selectRandomWord(serializedWords);
        if (word.isEmpty())
        {
            Log.w("EVAlarm", "No vocabulary word is available for the notification.");
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

        Intent openWordIntent =
            new Intent(context, EVVocabularyAlarmReceiver.class)
                .setAction(ACTION_OPEN_WORD)
                .putExtra(EXTRA_NOTIFICATION_WORD, word);

        PendingIntent contentIntent = PendingIntent.getBroadcast(
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

        builder
            .setSmallIcon(context.getApplicationInfo().icon)
            .setContentTitle("Enhance Vocabulary")
            .setContentText(word)
            .setStyle(new Notification.BigTextStyle().bigText(word))
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

        notificationManager.notify(NOTIFICATION_ID, builder.build());
        Log.i("EVAlarm", "Vocabulary notification posted: " + word);
    }

    private static String selectRandomWord(String serializedWords)
    {
        if (serializedWords == null || serializedWords.isEmpty())
        {
            return "";
        }

        String[] rawWords = serializedWords.split("\\n");
        List<String> words = new ArrayList<>();

        for (String rawWord : rawWords)
        {
            if (rawWord != null)
            {
                String word = rawWord.trim();
                if (!word.isEmpty())
                {
                    words.add(word);
                }
            }
        }

        if (words.isEmpty())
        {
            return "";
        }

        return words.get(new Random().nextInt(words.size()));
    }
}

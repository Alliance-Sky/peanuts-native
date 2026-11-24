// PEANUTS: new file (the stats page's frame rate cap for a game on the frame limit list whose cap applies at the next start)
package dev.nxnative.android;

/**
 * Star Ocean The Second Story R and Hogwarts Legacy (opt-in present pacing: fps_limit_present with no fps_limit, Game owned by default) on the JVM: the stats page
 * offers their 60 fps, a 60 picked while the game runs Game owned waits for the next start (so the pop-up opens again with its note), the stored value keeps its
 * meaning at that start (the list marker, Prefs.migrateFpsCap), and FrameRateCaps.savedIn finds it in a settings file as Android writes one. Prints "ok" or throws
 */
public final class StatsCapJvmTest {
    private static void check(boolean c, String what) { if (!c) throw new AssertionError(what); }

    /** A profile line as build.gradle writes it: columns 17 (fps_limit_present), 31 (fps_limit_max), 35 (fps_modes) */
    static Profile line(String id, String name, String max, String modes) {
        String[] c = new String[43];
        java.util.Arrays.fill(c, "");
        c[0] = id; c[1] = name; c[2] = "0"; c[3] = "x"; c[4] = "v0";
        c[17] = "1"; c[31] = max; c[35] = modes;
        return Profile.parse(String.join("\t", c)).get(0);
    }

    public static void main(String[] args) {
        Profile so2r = line("010065301A2E0000", "STAR OCEAN THE SECOND STORY R", "", "20,30,60");
        Profile hogwarts = line("0100F7E00C70E000", "Hogwarts Legacy", "60", "");
        for (Profile p : new Profile[] {so2r, hogwarts}) {
            String n = p.name;
            check(FrameRateCaps.limited(p) && p.optInPresentRate(), n + ": the frame limit list, Game owned by default");
            check(FrameRateCaps.contains(FrameRateCaps.menu(), 0) && FrameRateCaps.contains(FrameRateCaps.paceOffered(p, 60), 0), n + ": the stats page offers 60 fps (stored 0)");
            check(p.fpsRestart(FrameRateCaps.GAME_OWNED, 0), n + ": a 60 picked while the game runs Game owned applies at the next start");
            check(!p.fpsRestart(0, 30), n + ": 60 to 30 applies at once");
            // the pick as Prefs.fpsCap stores it (the cap and the list marker), read at the next start: nothing to move, the 60 stays
            check(FrameRateCaps.migrateSaved(p, FrameRateCaps.list(p), 0) == null, n + ": the stored 60 is kept at the next start");
            check(FrameRateCaps.normalize(0, FrameRateCaps.paceMax(p)) == 0, n + ": the stored 0 reads as 60");
        }
        check(so2r.fpsMode(0) == 0 && so2r.fpsMode(FrameRateCaps.GAME_OWNED) == FrameRateCaps.GAME_OWNED, "Star Ocean: 60 and Game owned are its modes");

        // the settings file as Android writes it (XmlUtils.writeMapXml)
        String xml = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>\n<map>\n    <string name=\"fps_cap_list@010065301A2E0000\">limit</string>\n"
                + "    <int name=\"fps_cap@010065301A2E0000\" value=\"0\" />\n    <int name=\"fps_cap@0100F7E00C70E000\" value=\"-2\" />\n</map>\n";
        check(FrameRateCaps.savedIn(xml, "fps_cap@010065301A2E0000", 0), "the stored 60 is found");
        check(FrameRateCaps.savedIn(xml, "fps_cap@0100F7E00C70E000", FrameRateCaps.GAME_OWNED), "a stored Game owned is found");
        check(!FrameRateCaps.savedIn(xml, "fps_cap@0100F7E00C70E000", 0), "another value is not the pick");
        check(!FrameRateCaps.savedIn(xml, "fps_cap@01006F8002326000", 0), "another game's key is not the pick");
        check(!FrameRateCaps.savedIn(null, "fps_cap@010065301A2E0000", 0), "a file not read holds nothing");
        System.out.println("ok");
    }
}

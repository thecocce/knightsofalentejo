package br.odb.knights;

import android.content.res.Resources;

import br.odb.droidlib.Vector2;

class Baphomet extends Monster {

    public Baphomet(Resources res) {
        super(R.drawable.boss, 100, 1, res);
    }

    @Override
    public void updateTarget(GameLevel level) {
        Vector2 myPosition = getPosition();
        Vector2 scan = new Vector2();
        int newX;
        int newY;

        for (int x = -10; x < 10; ++x) {
            for (int y = -10; y < 10; ++y) {

                newX = (int) (x + myPosition.x);
                newY = (int) (y + myPosition.y);
                scan.x = newX;
                scan.y = newY;

                if (newX >= 0 && newY >= 0 && newX < GameLevel.BASE_SQUARE_SIDE
                        && newY < GameLevel.BASE_SQUARE_SIDE
                        && level.getTile(scan).getOccupant() instanceof Knight) {

                    if (dealWith(level, x, y))
                        return;
                }
            }
        }
    }


    @Override
    public String getChar() {
        return String.valueOf(KnightsConstants.SPAWNPOINT_BAPHOMET);
    }

    @Override
    public int getTextureIndex() {
        return GameScreenView.ETextures.Boss0.ordinal() + getStateFrame();
    }
}

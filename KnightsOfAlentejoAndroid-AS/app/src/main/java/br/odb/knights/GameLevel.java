package br.odb.knights;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Region;

import java.util.ArrayList;

import br.odb.droidlib.Layer;
import br.odb.droidlib.Renderable;
import br.odb.droidlib.Tile;
import br.odb.droidlib.Updatable;
import br.odb.droidlib.Vector2;
import br.odb.menu.GameActivity;

public class GameLevel extends Layer {

    public static final int BASE_SQUARE_SIDE = 20;
    final private Tile[][] tileMap;
    final private ArrayList<Actor> entities;
    private int remainingMonsters;
    public Bitmap[] bitmaps;

    @Override
    public String toString() {

        String toReturn = "";

        for (Actor a : entities) {
            if (a.isAlive()) {

                toReturn += a.getStats();
            }
        }

        return toReturn;
    }

    @Override
    public void draw(Canvas canvas, Vector2 camera) {

        for (Renderable r : children) {
            canvas.clipRect(position.x, position.y, position.x + size.x,
                    position.y + size.y, Region.Op.UNION);

            if ((r instanceof Actor)) {
                continue;
            }
            r.draw(canvas, position.add(camera));
        }

        for (Renderable r : children) {
            canvas.clipRect(position.x, position.y, position.x + size.x,
                    position.y + size.y, Region.Op.UNION);

            if (r instanceof Actor) {

                if (((Actor) r).isAlive()) {
                    continue;
                }

                r.draw(canvas, position.add(camera));
            }
        }

        for (Renderable r : children) {
            canvas.clipRect(position.x, position.y, position.x + size.x,
                    position.y + size.y, Region.Op.UNION);

            if (r instanceof Actor) {

                if (((Actor) r).isAlive()) {
                    r.draw(canvas, position.add(camera));
                }
            }
        }
    }

	@Override
	public GameScreenView.ETextures getTextureIndex() {
		return GameScreenView.ETextures.None;
	}

	public GameLevel(int[][] map, Resources res) {
        tileMap = new Tile[BASE_SQUARE_SIDE][BASE_SQUARE_SIDE];
        entities = new ArrayList<Actor>();
        int[] row;
        Tile tile;

        bitmaps = new Bitmap[]{
                BitmapFactory.decodeResource(res, R.drawable.grass),
                BitmapFactory.decodeResource(res, R.drawable.bricks),
                BitmapFactory.decodeResource(res, R.drawable.bricks),
                BitmapFactory.decodeResource(res, R.drawable.falcon),
                BitmapFactory.decodeResource(res, R.drawable.turtle),
                BitmapFactory.decodeResource(res, R.drawable.cuco),
                BitmapFactory.decodeResource(res, R.drawable.lady),
                BitmapFactory.decodeResource(res, R.drawable.demon),
                BitmapFactory.decodeResource(res, R.drawable.boss),
                BitmapFactory.decodeResource(res, R.drawable.begin),
                BitmapFactory.decodeResource(res, R.drawable.exit),
                BitmapFactory.decodeResource(res, R.drawable.bricks_blood),
                BitmapFactory.decodeResource(res, R.drawable.bricks_candles),
                BitmapFactory.decodeResource(res, R.drawable.bars),
                BitmapFactory.decodeResource(res, R.drawable.arch)};

        for (int c = 0; c < map.length; ++c) {
            row = map[c];
            for (int d = 0; d < row.length; ++d) {

                switch (row[d]) {

                    case KnightsConstants.BARS:
                        tile = new Tile(c, d, row[d], bitmaps[13], GameScreenView.ETextures.Bars );
                        tile.setKind(row[d]);
                        tile.setBlock(true);
                        break;

                    case KnightsConstants.ARCH:
                        tile = new Tile(c, d, row[d], bitmaps[14], GameScreenView.ETextures.Arch );
                        tile.setBlock(false);
                        break;

                    case KnightsConstants.BRICKS_BLOOD:
                        tile = new Tile(c, d, row[d], bitmaps[11], GameScreenView.ETextures.BricksBlood );
                        tile.setBlock(true);
                        break;

                    case KnightsConstants.BRICKS_CANDLES:
                        tile = new Tile(c, d, row[d], bitmaps[12], GameScreenView.ETextures.BricksCandles );
                        tile.setBlock(true);
                        tile.setImage(bitmaps[12]);
                        break;

                    case KnightsConstants.BRICKS:
                        tile = new Tile(c, d, row[d], bitmaps[1], GameScreenView.ETextures.Bricks );
                        tile.setBlock(true);
                        break;

                    case KnightsConstants.DOOR:
                        tile = new Tile(c, d, row[d], bitmaps[10], GameScreenView.ETextures.Exit );
                        tile.setBlock(false);
                        break;
                    case KnightsConstants.BEGIN:
                        tile = new Tile(c, d, row[d], bitmaps[9], GameScreenView.ETextures.Begin );
                        tile.setBlock(true);
                        break;
                    default:
                        tile = new Tile(c, d, row[d], bitmaps[0], GameScreenView.ETextures.Grass );
                }
                this.add(tile);
                this.tileMap[c][d] = tile;
            }
        }
    }

    public void tick() {
        Monster m;
	    int monstersBefore = remainingMonsters;

        remainingMonsters = 0;
        for (Actor a : entities) {
            if (a instanceof Monster && a.isAlive()) {
                m = (Monster) a;
                m.updateTarget(this);
                ++remainingMonsters;
            }
        }
	    GameConfigurations.getInstance().getCurrentGameSession().addtoScore( monstersBefore - remainingMonsters);
    }

    public void reset(Resources res) {
        int kind;
        for (int c = 0; c < tileMap.length; ++c) {
            for (int d = 0; d < tileMap[c].length; ++d) {

                kind = tileMap[c][d].getKind();

                switch (kind) {

                    case KnightsConstants.SPAWNPOINT_BAPHOMET:
                        addEntity(new Baphomet(res), c, d);
                        ++remainingMonsters;
                        break;
                    case KnightsConstants.SPAWNPOINT_BULL:
                        addEntity(new BullKnight(res), c, d);
                        break;
                    case KnightsConstants.SPAWNPOINT_TURTLE:
                        addEntity(new TurtleKnight(res), c, d);
                        break;
                    case KnightsConstants.SPAWNPOINT_EAGLE:
                        addEntity(new EagleKnight(res), c, d);
                        break;
                    case KnightsConstants.SPAWNPOINT_CUCO:
                        addEntity(new Cuco(res), c, d);
                        ++remainingMonsters;
                        break;
                    case KnightsConstants.SPAWNPOINT_MOURA:
                        addEntity(new Moura(res), c, d);
                        ++remainingMonsters;
                        break;
                    case KnightsConstants.SPAWNPOINT_DEVIL:
                        addEntity(new Demon(res), c, d);
                        ++remainingMonsters;
                        break;
                }
            }
        }
    }

    private void addEntity(Actor actor, int c, int d) {
        add(actor);
        entities.add(actor);
        tileMap[c][d].setOccupant(actor);
        actor.setPosition(new Vector2(c, d));
    }

    public Tile getTile(Vector2 position) {
        return this.tileMap[(int) position.x][(int) position.y];
    }

    public int getTotalActors() {
        return entities.size();
    }

    public Updatable getActor(int c) {
        return entities.get(c);
    }

    public boolean validPositionFor(Actor actor) {

        int c, d;
        c = (int) actor.getPosition().x;
        d = (int) actor.getPosition().y;

        if (tileMap[c][d].isBlock()) {
	        return false;
        }

        if ((tileMap[c][d].getOccupant() instanceof Actor)
                && !((Actor) tileMap[c][d].getOccupant()).isAlive()) {
	        return true;
        }

        if ((tileMap[c][d].getOccupant() instanceof Knight)
                && ((Knight) tileMap[c][d].getOccupant()).hasExited) {
	        return true;
        }

        return !(tileMap[c][d].getOccupant() instanceof Actor);
    }

    private Actor getActorAt(int x, int y) {

        if (tileMap[x][y].getOccupant() instanceof Actor)
            return ((Actor) tileMap[x][y].getOccupant());
        else
            return null;
    }

    public void battle(Actor attacker, Actor defendant) {

        Vector2 pos;

        attacker.attack(defendant);
        defendant.attack(attacker);

        if (!attacker.isAlive()) {

            pos = attacker.getPosition();
            tileMap[(int) pos.x][(int) pos.y].setOccupant(null);
        }

        if (!defendant.isAlive()) {

            pos = defendant.getPosition();
            tileMap[(int) pos.x][(int) pos.y].setOccupant(null);
        }
    }

    public int getScreenWidth() {

        return BASE_SQUARE_SIDE * tileMap[0][0].getWidth();
    }

    public int getScreenHeight() {

        return BASE_SQUARE_SIDE * tileMap[0][0].getHeight();
    }

    public Actor getActorAt(Vector2 position) {

        return getActorAt((int) position.x, (int) position.y);
    }

    public Knight[] getKnights() {
        ArrayList<Knight> knights_filtered = new ArrayList<Knight>();

        for (Actor a : entities) {
            if (a instanceof Knight && a.isAlive() && !((Knight) a).hasExited) {
                knights_filtered.add((Knight) a);
            }
        }

        Knight[] knights = new Knight[knights_filtered.size()];
        return knights_filtered.toArray(knights);
    }

    public int getMonsters() {
        return remainingMonsters;
    }

    public boolean isBlockAt(int x, int y) {
        return tileMap[x][y].isBlock();
    }

    public int getLevelNumber() {
        return 0;
    }

    public boolean canMove(Actor actor, GameActivity.Direction direction) {
        Vector2 position = actor.getPosition().add( direction.getOffsetVector());

        return !isBlockAt( (int)position.x, (int)position.y );
    }

    public boolean canAttack(Actor actor, GameActivity.Direction direction) {
        Vector2 position = actor.getPosition().add( direction.getOffsetVector());
        return getActorAt( (int)position.x, (int)position.y ) instanceof Monster;
    }
}

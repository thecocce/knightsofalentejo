package br.odb.knights;

import android.content.res.Resources;


public class BullKnight extends Knight {
	public BullKnight( Resources res ) {
		super( R.drawable.bull, 20, 14, res );
	}
	
	@Override
	public String toString() {
	
		return "Bull Knight - " + super.toString();
	}

	@Override
	public String getChar() {
		return String.valueOf( KnightsConstants.SPAWNPOINT_BULL );
	}
}
